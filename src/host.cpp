/*
 *  This file is part of the Xen Crashdump Analyser.
 *
 *  The Xen Crashdump Analyser is free software: you can redistribute
 *  it and/or modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation, either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  The Xen Crashdump Analyser is distributed in the hope that it will
 *  be useful, but WITHOUT ANY WARRANTY; without even the implied
 *  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the Xen Crashdump Analyser.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2011,2012 Citrix Inc.
 */

/**
 * @file src/host.cpp
 * @author Andrew Cooper
 */

#include "host.hpp"

#include "Xen.h"

#include "abstract/xensyms.hpp"

#include "arch/x86_64/pagetable-walk.hpp"
#include "arch/x86_64/pcpu.hpp"
#include "arch/x86_64/domain.hpp"
#include "arch/x86_64/xensyms.hpp"

#include "util/print-structures.hpp"
#include "util/log.hpp"
#include "memory.hpp"
#include "util/file.hpp"
#include "util/macros.hpp"
#include "util/stdio-wrapper.hpp"

#include <new>
#include <sysexits.h>
#include <errno.h>

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <string.h> // For strndup

using namespace Abstract::xensyms;
using namespace x86_64::xensyms;

Host::Host():
    once(false), arch(Abstract::Elf::ELF_Unknown), nr_pcpus(0),
    pcpus(NULL), idle_vcpus(NULL), pcpu_stacks(NULL),
    symtab(), dom0_symtab(),
    active_vcpus(),
    xen_major(0), xen_minor(0), xen_extra(NULL),
    xen_changeset(NULL), xen_compiler(NULL),
    xen_compile_date(NULL), debug_build(false),
    can_validate_xen_vaddr(false), xen_vmcoreinfo(), dom0_vmcoreinfo()
{}

Host::~Host()
{
    if ( this -> pcpus )
    {
        for ( int i = 0; i < this->nr_pcpus; ++i )
            SAFE_DELETE(this->pcpus[i]);
        delete [] this -> pcpus;
        this -> pcpus = NULL;
    }

    SAFE_DELETE_ARRAY(this->idle_vcpus);
    SAFE_DELETE_ARRAY(this->pcpu_stacks);
    SAFE_DELETE_ARRAY(this->xen_extra);
    SAFE_DELETE_ARRAY(this->xen_changeset);
    SAFE_DELETE_ARRAY(this->xen_compiler);
    SAFE_DELETE_ARRAY(this->xen_compile_date);
}

bool Host::setup(const Abstract::Elf * elf)
{
    if ( this -> once )
        return false;
    this->once = true;

    if ( elf->arch != Abstract::Elf::ELF_64 )
    {
        LOG_ERROR("TODO - implement decoding for non-64bit Xen\n");
        return false;
    }

    this->arch = elf->arch;
    this->nr_pcpus = elf->nr_cpus;

    try
    {
        this->pcpus = new Abstract::PCPU*[nr_pcpus];

        for ( int x = 0; x < nr_pcpus; ++x )
            if ( arch == Abstract::Elf::ELF_64 )
                this->pcpus[x] = new x86_64::PCPU();
            else
            {
                // Implement if necessary
            }

        this->idle_vcpus = new vaddr_t[nr_pcpus];
        this->pcpu_stacks = new vaddr_t[nr_pcpus];

        for ( int x = 0; x < nr_pcpus; ++x )
            this->idle_vcpus[x] = this->pcpu_stacks[x] = -((vaddr_t)1);
    }
    catch ( const std::bad_alloc & )
    {
        LOG_ERROR("Bad alloc for PCPUs.  Kdump environment needs more memory\n");
        return false;
    }

    int pt_index = 0, xen_core_index = 0;
    for ( int x = 0; x < elf->nr_notes; ++x )
        switch ( elf->notes[x].type )
        {
        case NT_PRSTATUS:
            this->pcpus[pt_index]->parse_pr_status(
                elf->notes[x].desc, elf->notes[x].desc_size, pt_index);
            ++pt_index;
            break;
        case XEN_ELFNOTE_CRASH_INFO:
            this->parse_crash_xen_info(elf->notes[x].desc, elf->notes[x].desc_size);
            break;
        case XEN_ELFNOTE_CRASH_REGS:
            this->pcpus[xen_core_index]->parse_xen_crash_core(
                elf->notes[x].desc, elf->notes[x].desc_size, xen_core_index);
            ++xen_core_index;
            break;
        case XEN_ELFNOTE_VMCOREINFO:
            this->parse_vmcoreinfo(elf->notes[x]);
            break;
        default:
            break;
        }

    /* If any pcpus are online (i.e. successfully got PRSTATUS and CRASH_REGS),
       consider setup a success... */
    for ( int x = 0; x < nr_pcpus; ++x )
        if ( this->pcpus[x]->is_online() )
            return true;

    /* ...But if all pcpus are offline then consider setup a failure. */
    return false;
}

bool Host::parse_crash_xen_info(const char * buff, const size_t len)
{
    char * tmp = NULL;

    if ( arch != Abstract::Elf::ELF_64 )
    {
        LOG_ERROR("TODO - implement decoding for non-64bit Xen\n");
        return false;
    }

    x86_64_crash_xen_info_t * info = (x86_64_crash_xen_info_t*)buff;

    if ( len != sizeof *info )
    {
        LOG_ERROR("Wrong size for crash_xen_info note.  Expected %zu, got %zu\n",
                  sizeof *info, len);
        return false;
    }

    try
    {
        this->xen_major = (int)info->xen_major_version;
        this->xen_minor = (int)info->xen_minor_version;

        tmp = new char[1024];
        tmp[0] = 0;

/// @cond EXCLUDE
#define GET_STR(src, dst) do {                              \
            size_t sz = memory.read_str((src), tmp, 1023);  \
            (dst) = new char [ sz+1 ];                      \
            strcpy((dst), tmp); (dst)[sz]=0; } while (0)

        GET_STR(info->xen_extra_version, this->xen_extra);
        GET_STR(info->xen_changeset, this->xen_changeset);
        GET_STR(info->xen_compiler, this->xen_compiler);
        GET_STR(info->xen_compile_date, this->xen_compile_date);
#undef GET_STR
/// @endcond

    }
    catch ( const std::bad_alloc & )
    {
        LOG_ERROR("Bad alloc for PCPUs.  Kdump environment needs more memory\n");
    }
    catch ( const CommonError & e )
    {
        e.log();
    }

    SAFE_DELETE_ARRAY(tmp);

    return true;
}


bool Host::decode_xen()
{
    LOG_INFO("Decoding physical CPU information.  %d PCPUs\n", this->nr_pcpus);

    if ( ! ( REQ_x86_64_XENSYMS(x86_64_per_cpu) &
             REQ_CORE_XENSYMS(misc) ))
        return false;

    try
    {
        this->debug_build = XEN_DEBUG;
        const Abstract::PageTable & xenpt = this->get_xenpt();

        if ( this->debug_build )
            LOG_DEBUG("Xen is a debug build.  Will adjust for poisoned registers.\n");

        if ( this->arch == Abstract::Elf::ELF_64 )
        {
            LOG_DEBUG("  Reading per-pcpu information\n");
            for ( int x = 0; x < this->nr_pcpus; ++x )
            {
                vaddr_t idle = idle_vcpu + (x * sizeof(uint64_t) );
                host.validate_xen_vaddr(idle);
                memory.read64_vaddr(xenpt, idle, this->idle_vcpus[x]);

                vaddr_t stack = stack_base + (x * sizeof(uint64_t));
                host.validate_xen_vaddr(stack);
                memory.read64_vaddr(xenpt, stack, this->pcpu_stacks[x]);
            }
        }
        else
        {
            // Implement if necessary
        }

        LOG_DEBUG("  Reading PCPUs vcpus\n");
        for (int x=0; x < nr_pcpus; ++x)
        {
            if ( ! this->pcpus[x]->is_online() )
            {
                LOG_DEBUG("  Skipping pcpu%d - offline\n", x);
                continue;
            }
            if ( ! this->pcpus[x]->decode_extended_state() )
                LOG_WARN("  Failed to decode extended state for pcpu%d\n", x);
        }

        this->active_vcpus.reserve(nr_pcpus);
        LOG_DEBUG("  Generating active vcpu list\n");

        for (int x=0; x < nr_pcpus; ++x)
            switch ( this->pcpus[x]->vcpu_state )
            {
            case Abstract::PCPU::CTX_IDLE:
            case Abstract::PCPU::CTX_RUNNING:
                this->active_vcpus.push_back(
                    vcpu_pair(this->pcpus[x]->vcpu->vcpu_ptr,
                              this->pcpus[x]->vcpu));
                break;
            case Abstract::PCPU::CTX_SWITCH:
                this->active_vcpus.push_back(
                    vcpu_pair(this->pcpus[x]->ctx_from->vcpu_ptr,
                              this->pcpus[x]->ctx_from));
                break;
            case Abstract::PCPU::CTX_UNKNOWN:
            case Abstract::PCPU::CTX_NONE:
                break;
            }

        return true;
    }
    catch ( const std::bad_alloc & )
    {
        LOG_ERROR("Bad alloc for PCPUs.  Kdump environment needs more memory\n");
    }
    catch ( const CommonError & e )
    {
        e.log();
    }

    return false;
}

bool Host::print_xen(bool dump_structures)
{
    static const char * xen_log_file = "xen.log";
    int len = 0;
    bool success = false;
    char * cmdline = NULL;
    FILE * o = NULL;

    // Try to open the xen.log file
    if ( NULL == (o = fopen_in_outdir(xen_log_file, "w")))
    {
        LOG_ERROR("Unable to open %s in output directory: %s\n",
                  xen_log_file, strerror(errno));
        return false;
    }
    LOG_INFO("Opened for host information\n", xen_log_file);

    set_additional_log(o);

    try
    {
        const Abstract::PageTable & xenpt = this->get_xenpt();

        // Print some header information for the host
        if ( this->xen_extra )
            len += FPRINTF(o, "Xen version:      %d.%d%s\n", this->xen_major,
                           this->xen_minor, this->xen_extra);
        if ( this->xen_changeset )
            len += FPRINTF(o, "Xen changeset:    %s\n", this->xen_changeset);
        if ( this->xen_compiler )
            len += FPRINTF(o, "Xen compiler:     %s\n", this->xen_compiler);
        if ( this->xen_compile_date )
            len += FPRINTF(o, "Xen compile date: %s\n", this->xen_compile_date);

        len += FPRINTF(o, "Debug build:      %s\n\n",
                       this->debug_build ? "true" : "false");

        // Try to find and print the saved command line string
        const Symbol * cmdline_sym = this->symtab.find("saved_cmdline");
        if ( ! cmdline_sym )
            len += FPUTS("Missing symbol for command line\n", o);
        else
        {
            try
            {
                // Size hardcoded in Xen
                cmdline = new char[1024];

                host.validate_xen_vaddr(cmdline_sym->address);
                memory.read_str_vaddr(xenpt, cmdline_sym->address, cmdline, 1023);
                len += FPRINTF(o, "Xen command line: %s\n", cmdline);

                SAFE_DELETE_ARRAY(cmdline);
            }
            catch ( const std::bad_alloc & )
            {
                LOG_ERROR("Bad Alloc exception.  Out of memory\n");
            }
            catch ( const CommonError & e )
            {
                e.log();
            }
            SAFE_DELETE_ARRAY(cmdline);
        }

        len += FPUTS("\n", o);

        // Dump the Xen vmcoreinfo, if set
        if ( this->xen_vmcoreinfo.vmcoreinfoData() != NULL )
        {
            len += FPRINTF(o, "VMCOREINFO:\n%s",
                    this->xen_vmcoreinfo.vmcoreinfoData());
            this->xen_vmcoreinfo.destroy(); // Don't need it any more
            len += FPUTS("\n", o);
        }

        for (int x=0; x < nr_pcpus; ++x)
            len += this->pcpus[x]->print_state(o);

        len += FPUTS("\n  Console Ring:\n", o);

        if ( HAVE_CORE_XENSYMS(console) )
        {
            uint64_t conring_ptr,length;
            uint32_t tmp;

            host.validate_xen_vaddr(conring);
            host.validate_xen_vaddr(conring_size);

            memory.read64_vaddr(xenpt, conring, conring_ptr);
            memory.read32_vaddr(xenpt, conring_size, tmp);
            length = tmp;

            if ( HAVE_CORE_XENSYMS(consolepc) )
            {
                uint64_t prod,cons;

                host.validate_xen_vaddr(conringp);
                host.validate_xen_vaddr(conringc);

                memory.read32_vaddr(xenpt, conringp, tmp);
                prod = tmp;
                memory.read32_vaddr(xenpt, conringc, tmp);
                cons = tmp;

                len += print_console_ring(o, xenpt, conring_ptr, length, prod, cons);
            }
            else
                len += print_console_ring(o, xenpt, conring_ptr, length, 0, 0);
        }
        else
            len += FPUTS("    Missing conring symbols\n", o);

        success = true;
    }
    catch ( const CommonError & e )
    {
        e.log();
    }
    catch ( const filewrite & e )
    {
        e.log(xen_log_file);
    }

    set_additional_log(NULL);
    SAFE_FCLOSE(o);

    // If we dont wish to dump the structures, return now
    if ( ! dump_structures )
        return success;

    for (int x=0; x < nr_pcpus; ++x)
    {
        char filename[32];
        FILE * file;

        if ( !this->pcpus[x]->is_online() || this->pcpus[x]->processor_id != x )
            continue;

        if ( snprintf(filename, sizeof filename, "xen.pcpu%d.stack.log", x) < 0 )
            continue;

        if ( NULL == (file = fopen_in_outdir(filename, "w")) )
        {
            LOG_ERROR("Unable to open %s in output directory: %s\n",
                      filename, strerror(errno));
            continue;
        }

        set_additional_log(file);
        this->pcpus[x]->dump_stack(file);
        set_additional_log(NULL);
        SAFE_FCLOSE(file);
    }

    return success;
}

int Host::print_domains(bool dump_structures)
{
    int success = 0;

    LOG_INFO("Decoding Domains\n");

    if ( ! REQ_CORE_XENSYMS(domain) )
        return success;

    if ( this->arch != Abstract::Elf::ELF_64 )
    {
        // Implement if necessary
        LOG_ERROR("TODO - implement decoding for non-64bit Xen\n");
        return success;
    }

    Abstract::Domain * dom = NULL;
    vaddr_t dom_ptr;
    FILE * fd = NULL;
    static char fname[32] = { 0 };

    try
    {
        const Abstract::PageTable & xenpt = this->get_xenpt();

        host.validate_xen_vaddr(domain_list);
        memory.read64_vaddr(xenpt, domain_list, dom_ptr);
        LOG_DEBUG("  Domain pointer = 0x%016"PRIx64"\n", dom_ptr);

        while ( dom_ptr )
        {
            dom = new x86_64::Domain(xenpt);

            host.validate_xen_vaddr(dom_ptr);
            if ( ! dom->parse_basic(dom_ptr) )
            {
                LOG_WARN("  Failed to parse domain basics.  Cant continue with this domain\n");
                break;
            }

            /* Update dom_ptr as early as possible so we can continue around
             * this loop in the case of semi-recoverable failures.  The pointer
             * itself will be validated at the top of the next loop, so we get
             * a chance to print this information.
             */
            dom_ptr = dom->next_domain_ptr;
            LOG_INFO("  Found domain %"PRIu16"\n", dom->domain_id);

            snprintf(fname, sizeof fname, "dom%d.log", dom->domain_id);
            if ( ! (fd = fopen_in_outdir(fname, "w")) )
            {
                LOG_ERROR("    Failed to open file '%s' in output directory\n",
                          fname);
                goto loop_cont;
            }
            LOG_DEBUG("    Logging to '%s'\n", fname);

            /* As we have opened the file, might as well log errors to their
             * relevant context.
             */
            set_additional_log(fd);

            if ( ! dom->parse_vcpus_basic() )
            {
                LOG_ERROR("    Failed to parse basic cpu information for domain %d\n",
                          dom->domain_id);
                goto loop_cont;
            }

            /* Try to match up this domains vcpus with vcpus running or idle on
             * Xen's pcpus.  If so, take the up-to-date register state.
             */
            for ( uint32_t v = 0; v < dom->max_cpus; v++ )
            {
                unsigned int p; bool found;

                if ( ! dom->vcpus[v]->is_online() )
                {
                    LOG_DEBUG("    Dom%"PRIu16" vcpu%"PRIu32" was not up\n", dom->domain_id, v);
                    continue;
                }

                for (p = 0, found = false; p < this->active_vcpus.size(); p++)
                    if ( this->active_vcpus[p].first == dom->vcpus[v]->vcpu_ptr )
                    {
                        found = true;
                        break;
                    }

                if ( found )
                {
                    LOG_DEBUG("    Dom%"PRIu16" vcpu%"PRIu32" was active on pcpu%u\n",
                              dom->domain_id, v, p);
                    dom->vcpus[v]->copy_from_active(this->active_vcpus[p].second);
                }
                else
                {
                    LOG_DEBUG("    Dom%"PRIu16" vcpu%"PRIu32" was not active\n",
                              dom->domain_id, v);
                    dom->vcpus[v]->runstate = Abstract::VCPU::RST_NONE;
                    dom->vcpus[v]->parse_extended(xenpt);
                }
            }

            try
            {
                dom->print_state(fd);
            }
            catch ( const filewrite & e )
            {
                e.log(fname);
            }

            // We are going to dump the xen structures...
            if ( dump_structures )
            {
                // so start off by cleaning up
                set_additional_log(NULL);
                SAFE_FCLOSE(fd);

                // and open up some newer files
                snprintf(fname, sizeof fname, "dom%d.structures.log", dom->domain_id);
                if ( ! (fd = fopen_in_outdir(fname, "w")) )
                {
                    LOG_ERROR("    Failed to open file '%s' in output directory\n",
                              fname);
                    goto loop_cont;
                }
                LOG_DEBUG("    Dumping structures to '%s'\n", fname);
                set_additional_log(fd);

                try
                {
                    dom->dump_structures(fd);
                }
                catch ( const filewrite & e )
                {
                    e.log(fname);
                }
            }

            ++success;

        loop_cont:
            set_additional_log(NULL);
            SAFE_FCLOSE(fd);
            SAFE_DELETE(dom);
        }
    }
    catch ( const std::bad_alloc & )
    {
        LOG_ERROR("Bad Alloc exception.  Out of memory\n");
    }
    catch ( const CommonError & e )
    {
        e.log();
    }

    set_additional_log(NULL);
    SAFE_FCLOSE(fd);
    SAFE_DELETE(dom);

    return success;
}

bool Host::validate_xen_vaddr(const vaddr_t & vaddr, const bool except)
{
    /* If we didn't find the information in the Xen symbol table, assume
     * valid.  It is better than blindly failing. In main(), this is detected
     * and a warning issued. */
    if ( ! this->can_validate_xen_vaddr )
        return true;

    // Xen .text, .bss etc
    if ( VIRT_XEN_START <= vaddr && vaddr < VIRT_XEN_END )
        return true;
    // 1:1 direct mapping of physical memory
    else if ( VIRT_DIRECTMAP_START <= vaddr && vaddr < VIRT_DIRECTMAP_END )
        return true;

    if ( except )
        throw validate(vaddr, "Not in Xen Virtual Address regions.");
    return false;
}

const Abstract::PageTable & Host::get_xenpt() const
{
    if ( ! this->pcpus )
        throw validate(0, "No suitable PCPUs.");

    for ( int i = 0; i < this->nr_pcpus; ++i )
        if ( this->pcpus[i] && this->pcpus[i]->xenpt )
            return *this->pcpus[i]->xenpt;
    throw validate(0, "No suitable PCPU Xen pagetables.");
}

bool Host::parse_vmcoreinfo(const ElfNote& note)
{
    /* N.B. Both Xen and dom0 vmcoreinfo ELF notes use the same
     * note type. The difference is encoded in the note name:
     * VMCOREINFO     : dom0
     * VMCOREINFO_XEN : Xen
     *
     * The note name can be taken as the note.name_size bytes
     * starting from note.name
     *
     * The note data to be copied is only the relevant part of
     * note.desc, which will end either with a null character
     * (from the ELF section padding) or be at the end of the
     * note itself. Since vmcoreinfo data is newline-separated,
     * this can be checked by testing the last byte of note.desc
     * for '\0'. If it is, strlen is safe to use. Otherwise,
     * assume the data occupies the whole note.
     */
    size_t data_size = 0;
    if ( note.desc[note.desc_size-1] == '\0' )
        data_size = strlen(note.desc);
    else
        data_size = note.desc_size;

    try
    {
        CoreInfo info(note.name, note.name_size, note.desc, data_size);
        // search for XEN in note name to see which vmcoreinfo we have
        if ( strcmp(info.vmcoreinfoName(), "VMCOREINFO_XEN") == 0 )
            this->xen_vmcoreinfo.transferOwnershipFrom(info);
        else
            this->dom0_vmcoreinfo.transferOwnershipFrom(info);
        return true;
    }
    catch ( const std::bad_alloc & )
    {
        LOG_ERROR("Bad alloc for VMCOREINFO. Out of memory.\n");
        return false;
    }
}

/// Host container
Host host;

/*
 * Local variables:
 * mode: C++
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
