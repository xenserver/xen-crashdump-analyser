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
 *  Copyright (c) 2012 Citrix Inc.
 */

#include "arch/x86_64/domain.hpp"
#include "arch/x86_64/vcpu.hpp"
#include "arch/x86_64/xensyms.hpp"
#include "abstract/xensyms.hpp"
#include "memory.hpp"
#include "host.hpp"
#include "util/print-structures.hpp"
#include "util/print-bitwise.hpp"
#include "util/log.hpp"
#include "util/macros.hpp"
#include "util/symbol.hpp"
#include "util/stdio-wrapper.hpp"

/**
 * @file src/arch/x86_64/domain.cpp
 * @author Andrew Cooper
 */

using namespace Abstract::xensyms;
using namespace x86_64::xensyms;

namespace x86_64
{

    Domain::Domain(const Abstract::PageTable & xenpt)
        : Abstract::Domain(xenpt)
    {
        memset(this->handle, 0, sizeof this->handle);
    }

    Domain::~Domain()
    {
        if ( this -> vcpus )
        {
            for ( uint32_t x = 0; x < this->max_cpus; ++x )
                SAFE_DELETE(this->vcpus[x]);
            this->max_cpus = 0;
            delete [] this->vcpus;
            this->vcpus = NULL;
        }
    }

    bool Domain::parse_basic(const vaddr_t & domain_ptr)
    {
        if ( ! ( REQ_CORE_XENSYMS(domain) &
                 REQ_x86_64_XENSYMS(x86_64_domain) ))
            return false;

        try
        {
            host.validate_xen_vaddr(domain_ptr);
            this->domain_ptr = domain_ptr;

            memory.read16_vaddr(this->xenpt, this->domain_ptr + DOMAIN_id, this->domain_id);

            memory.read8_vaddr (this->xenpt, this->domain_ptr + DOMAIN_is_32bit_pv, this->is_32bit_pv);
            memory.read8_vaddr (this->xenpt, this->domain_ptr + DOMAIN_is_hvm, this->is_hvm);
            memory.read8_vaddr (this->xenpt, this->domain_ptr + DOMAIN_is_privileged, this->is_privileged);

            memory.read32_vaddr(this->xenpt, this->domain_ptr + DOMAIN_max_vcpus, this->max_cpus);
            memory.read64_vaddr(this->xenpt, this->domain_ptr + DOMAIN_vcpus, this->vcpus_ptr);

            memory.read32_vaddr(this->xenpt, this->domain_ptr + DOMAIN_paging_mode, this->paging_mode);
            memory.read32_vaddr(this->xenpt, this->domain_ptr + DOMAIN_tot_pages, this->tot_pages);
            memory.read32_vaddr(this->xenpt, this->domain_ptr + DOMAIN_max_pages, this->max_pages);
            memory.read32_vaddr(this->xenpt, this->domain_ptr + DOMAIN_shr_pages, (uint32_t&)this->shr_pages);

            memory.read32_vaddr(this->xenpt, this->domain_ptr + DOMAIN_pause_count, this->pause_count);

            memory.read_block_vaddr(this->xenpt, this->domain_ptr + DOMAIN_handle,
                                    (char*)this->handle, sizeof this->handle);

            memory.read64_vaddr(this->xenpt, this->domain_ptr + DOMAIN_next, this->next_domain_ptr);

            return true;
        }
        catch ( const CommonError & e )
        {
            e.log();
        }

        return false;
    }

    bool Domain::parse_vcpus_basic()
    {
        try
        {
            host.validate_xen_vaddr(this->vcpus_ptr);

            if ( this->max_cpus < 1 )
            {
                LOG_ERROR("    No vcpus for domain\n");
                return false;
            }

            this->vcpus = new Abstract::VCPU*[this->max_cpus];
            std::memset(this->vcpus, 0, sizeof (Abstract::VCPU*) * this->max_cpus);

            LOG_INFO("    %"PRIu32" VCPUs\n", this->max_cpus);
            bool vcpus_online = false;

            for ( uint32_t x = 0; x < this->max_cpus; ++x )
            {
                vaddr_t vcpu_addr;
                this->vcpus[x] = new VCPU(Abstract::VCPU::RST_UNKNOWN);
                memory.read64_vaddr(this->xenpt, this->vcpus_ptr + x * 8, vcpu_addr);
                host.validate_xen_vaddr(vcpu_addr);
                LOG_DEBUG("    Vcpu%"PRIu32" pointer = 0x%016"PRIx64"\n", x, vcpu_addr);
                if ( this->vcpus[x]->parse_basic(vcpu_addr, this->xenpt) )
                    vcpus_online = true;
            }

            // If at least 1 vcpu is online, consider this successful
            return vcpus_online;
        }
        catch ( const std::bad_alloc & )
        {
            LOG_ERROR("Bad Alloc exception.  Out of memory\n");
        }
        catch ( const CommonError & e )
        {
            e.log();
        }

        return false;
    }

    bool Domain::read_vmcoreinfo(CoreInfo & dest) const
    {
        if ( this->domain_id != 0 )
            return false;
        /*
         * Find vmcoreinfo_note data:
         *  0           4           8           12
         *  _________________________________________________
         *  | name_len  | data_len  | note_type | V M C O   |
         *  | R E I N   | F O \0 \0 | ......... | ......... |
         *  | ......... | ......... | ......... | ......... |
         */
        const Symbol * note_sym = host.dom0_symtab.find("vmcoreinfo_note");
        if ( ! note_sym )
            return false;

        const Abstract::PageTable & dompt = this->get_dompt();
        uint32_t note_name_len(0), note_data_len(0), note_type(0);
        uint32_t max_data_size = 4096 - 24; // 1 page, minus (header + name)
        char name[12] = {0};

        try
        {
            memory.read32_vaddr(dompt, note_sym->address, note_name_len);
            memory.read32_vaddr(dompt, note_sym->address+4, note_data_len);
            memory.read32_vaddr(dompt, note_sym->address+8, note_type);

            // Validate the note header
            if ( note_name_len == 11 && note_data_len <= max_data_size
                 && note_type == 0 )
            {
                memory.read_str_vaddr(dompt, note_sym->address+12, name, 11);
                if (strncmp("VMCOREINFO", name, 10) == 0)
                {
                    CoreInfo tmp(10, note_data_len);
                    strncpy(tmp.vmcoreinfoName(), "VMCOREINFO", 10);
                    memory.read_block_vaddr(dompt, note_sym->address+24,
                            tmp.vmcoreinfoData(), note_data_len);
                    dest.transferOwnershipFrom(tmp);
                }
            }
            return true;
        }
        catch ( const CommonError & e )
        {
            e.log();
        }
        catch ( const std::bad_alloc & e )
        {
            LOG_ERROR("Bad Alloc exception.  Out of memory\n");
        }
        return false;
    }

    int Domain::print_vmcoreinfo(FILE * o, CoreInfo & info) const
    {
        int len(0);
        if ( info.vmcoreinfoData() )
            len += FPRINTF(o, "VMCOREINFO:\n%s\n", info.vmcoreinfoData());
        return len;
    }

    int Domain::print_state(FILE * o) const
    {
        int len = 0;

        len += FPRINTF(o, "Domain %"PRIu16": (%d vcpus)\n", this->domain_id, this->max_cpus);

        len += FPUTS("  Flags:", o);

        if ( this->is_privileged )
            len += FPUTS(" PRIVILEGED", o);

        if ( this->is_32bit_pv )
            len += FPUTS(" 32BIT-PV", o);

        if ( this->is_hvm )
            len += FPUTS(" HVM", o);

        if ( this->pause_count )
            len += FPRINTF(o, " PAUSED(count %"PRId32")", this->pause_count);
        else
            len += FPUTS(" UNPAUSED", o);

        len += FPUTS("\n", o);

        len += FPUTS("  Paging assistance: ", o);
        len += print_paging_mode(o, this->paging_mode);
        len += FPUTS("\n", o);

///@cond EXCLUDE
#define PAGES_TO_GB(p) (((double)(p)) * 4096.0 / (1024.0 * 1024.0 * 1024.0))
#define PAGES_TO_MB(p) (((double)(p)) * 4096.0 / (1024.0 * 1024.0))
#define PAGES_TO_KB(p) (((double)(p)) * 4096.0 / (1024.0))

        len += FPRINTF(o, "  Max Pages: %"PRIu32" (%.3fGB, %.3fMB, %.fKB)\n",
                       this->max_pages, PAGES_TO_GB(this->max_pages),
                       PAGES_TO_MB(this->max_pages), PAGES_TO_KB(this->max_pages));
        len += FPRINTF(o, "  Current Pages: %"PRIu32"\n", this->tot_pages);
        len += FPRINTF(o, "  Shared Pages: %"PRId32"\n", this->shr_pages);

        len += FPRINTF(o, "  Handle: %02"PRIx8"%02"PRIx8"%02"PRIx8"%02"PRIx8"-%02"PRIx8
                       "%02"PRIx8"-%02"PRIx8"%02"PRIx8"-""%02"PRIx8"%02"PRIx8"-%02"PRIx8
                       "%02"PRIx8"%02"PRIx8"%02"PRIx8"%02"PRIx8"%02"PRIx8"\n",
                       this->handle[ 0], this->handle[ 1], this->handle[ 2], this->handle[ 3],
                       this->handle[ 4], this->handle[ 5], this->handle[ 6], this->handle[ 7],
                       this->handle[ 8], this->handle[ 9], this->handle[10], this->handle[11],
                       this->handle[12], this->handle[13], this->handle[14], this->handle[15] );


        len += FPUTS("\n", o);

        CoreInfo vmcoreinfo;
        if ( this->domain_id == 0 )
        {
            len += this->print_cmdline(o);
            if ( this->read_vmcoreinfo(vmcoreinfo) )
                len += this->print_vmcoreinfo(o, vmcoreinfo);
        }

        for ( uint32_t x = 0; x < this->max_cpus; ++ x )
            if ( this->vcpus[x] )
            {
                len += FPRINTF(o, "  VCPU%"PRIu32":\n", this->vcpus[x]->vcpu_id);
                len += this->vcpus[x]->print_state(o);
            }
            else
                len += FPRINTF(o, "No information for vcpu%"PRIu32"\n", x);

        len += FPUTS("\n  Console Ring:\n", o);

        if ( this->domain_id == 0 )
            this->print_console(o, vmcoreinfo);
        else
            len += FPUTS("    No Symbol Table\n", o);

#undef PAGES_TO_KB
#undef PAGES_TO_MB
#undef PAGES_TO_GB
/// @endcond
        return len;
    }

    int Domain::dump_structures(FILE * o) const
    {
        int len = 0;

        if ( ! REQ_CORE_XENSYMS(domain) )
            return len;

        len += FPRINTF(o, "Xen structures for Domain %"PRId16"\n\n", this->domain_id);

        len += FPRINTF(o, "struct domain (0x%016"PRIx64")\n", this->domain_ptr);
        len += dump_64bit_data(o, this->xenpt, this->domain_ptr, DOMAIN_sizeof);

        for ( uint32_t x = 0; x < this->max_cpus; ++x )
            if ( this->vcpus[x] )
            {
                len += FPUTS("\n", o);
                len += this->vcpus[x]->dump_structures(o, this->xenpt);
            }
            else
                len += FPRINTF(o, "Nothing to dump for vcpu%"PRIu32"\n\n", x);

        return len;
    }

    int Domain::print_console(FILE * o, CoreInfo& info) const
    {
        int len = 0;

        // Sanity check until we support multiple domain symbol tables
        if ( this->domain_id != 0 )
            return len;

        const Symbol *log_end_sym, *log_buf_sym, *log_buf_len_sym;

        vaddr_t ring;
        uint64_t producer, length, consumer;
        uint32_t tmp;

        log_end_sym = host.dom0_symtab.find("log_end");
        log_buf_sym = host.dom0_symtab.find("log_buf");
        log_buf_len_sym = host.dom0_symtab.find("log_buf_len");

        if ( log_end_sym == NULL ||
             log_buf_sym == NULL || log_buf_len_sym == NULL )
        {
            if ( info.vmcoreinfoData() != NULL )
            {
                // Try to parse a linux 3.x log buffer
                len += print_console_3x(o, info);
            }

            if ( len == 0 )
            {
                len += FPUTS("\tUnavailable, the following symbols are not available:\n", o);
                len += FPRINTF(o, "  %s%s%s.\n\n",
                               log_end_sym     == NULL ? " log_end"     : "",
                               log_buf_sym     == NULL ? " log_buf"     : "",
                               log_buf_len_sym == NULL ? " log_buf_len" : "");
            }
            return len;
        }

        try
        {
            const Abstract::PageTable & dompt = this->get_dompt();

            if ( this->is_32bit_pv )
            {
                memory.read32_vaddr(dompt, log_buf_sym->address, tmp);
                ring = tmp;
            }
            else
                memory.read64_vaddr(dompt, log_buf_sym->address, ring);

            memory.read32_vaddr(dompt, log_end_sym->address, tmp);
            producer = tmp;

            memory.read32_vaddr(dompt, log_buf_len_sym->address, tmp);
            length = tmp;

            if ( length > (1<<21) )
            {
                len += FPRINTF(o, "\tLength of 0x%"PRIx64" looks abnormally long.  Truncating to"
                               "0x%x.\n", length, 1<<16);
                length = 1<<16;
            }

            consumer = producer > length ? producer - length : 0;

            len += print_console_ring(o, dompt, ring, length, producer, consumer);
        }
        catch ( const CommonError & e )
        {
            e.log();
        }

        return len;
    }

    int Domain::print_console_3x(FILE * o, CoreInfo& info) const
    {
        int len(0);
        vaddr_t log_buf_addr_addr, log_buf_len_addr;
        vaddr_t log_first_idx_addr, log_next_idx_addr;

        if ( ! info.lookup_key_vaddr("SYMBOL(log_buf)", log_buf_addr_addr) ||
             ! info.lookup_key_vaddr("SYMBOL(log_buf_len)", log_buf_len_addr) ||
             ! info.lookup_key_vaddr("SYMBOL(log_first_idx)", log_first_idx_addr) ||
             ! info.lookup_key_vaddr("SYMBOL(log_next_idx)", log_next_idx_addr) )
            return 0;

        try
        {
            const Abstract::PageTable & dompt = this->get_dompt();

            uint64_t log_buf_len(0), log_buf_addr(0);
            uint64_t log_first_idx(0), log_next_idx(0);
            uint32_t tmp(0);

            memory.read32_vaddr(dompt, log_buf_len_addr, tmp);
            log_buf_len = tmp;
            memory.read32_vaddr(dompt, log_first_idx_addr, tmp);
            log_first_idx = tmp;
            memory.read32_vaddr(dompt, log_next_idx_addr, tmp);
            log_next_idx = tmp;

            if ( this->is_32bit_pv )
            {
                memory.read32_vaddr(dompt, log_buf_addr_addr, tmp);
                log_buf_addr = tmp;
            }
            else
            {
                memory.read64_vaddr(dompt, log_buf_addr_addr, log_buf_addr);
            }

            vaddr_t log_buf = log_buf_addr;
            len += print_console_ring_3x(o, dompt, log_buf, log_buf_len,
                                  log_first_idx, log_next_idx);
        }
        catch ( const CommonError & e )
        {
            e.log();
        }

        return len;
    }

    int Domain::print_cmdline(FILE * o) const
    {
        int len = 0;
        char * cmdline = NULL;

        // Sanity check until we support multiple domain symbol tables
        if ( this->domain_id != 0 )
            return len;

        const Symbol * cmdline_sym = host.dom0_symtab.find("saved_command_line");
        if ( ! cmdline_sym )
            len += FPUTS("Missing symbol for command line\n", o);
        else
        {
            try
            {
                const Abstract::PageTable & dompt = this->get_dompt();

                // Size hardcoded in dom0
                cmdline = new char[2048];
                union { uint32_t val32; uint64_t val64; } cmdline_vaddr = {0};

                if ( this->is_32bit_pv )
                    memory.read32_vaddr(dompt, cmdline_sym->address, cmdline_vaddr.val32);
                else
                    memory.read64_vaddr(dompt, cmdline_sym->address, cmdline_vaddr.val64);

                memory.read_str_vaddr(dompt, cmdline_vaddr.val64, cmdline, 2047);
                len += FPRINTF(o, "  Command line: %s\n", cmdline);

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
        }

        len += FPUTS("\n", o);
        SAFE_DELETE_ARRAY(cmdline);
        return len;
    }

    const Abstract::PageTable & Domain::get_dompt() const
    {
        if ( ! this->vcpus )
            throw validate(0, "No suitable VCPUs.");

        for ( unsigned i = 0; i < this->max_cpus; ++i )
            if ( this->vcpus[i] && this->vcpus[i]->dompt )
                return *this->vcpus[i]->dompt;
        throw validate(0, "No suitable VCPU Domain pagetables.");
    }

}

/*
 * Local variables:
 * mode: C++
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
