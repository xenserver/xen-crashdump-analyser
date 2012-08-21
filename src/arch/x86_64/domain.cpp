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
#include "symbols.hpp"
#include "memory.hpp"
#include "host.hpp"
#include "util/print-structures.hpp"
#include "util/log.hpp"
#include "util/macros.hpp"
#include "util/symbol.hpp"

/**
 * @file src/arch/x86_64/domain.cpp
 * @author Andrew Cooper
 */

x86_64Domain::x86_64Domain()
{
    memset(this->handle, 0, sizeof this->handle);
}

x86_64Domain::~x86_64Domain()
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

bool x86_64Domain::parse_basic(const CPU & cpu, const vaddr_t & domain_ptr) throw ()
{
    if ( required_domain_symbols != 0 )
    {
        LOG_ERROR("Missing required domain symbols. %#x\n",
                  required_domain_symbols);
        return false;
    }

    try
    {
        host.validate_xen_vaddr(domain_ptr);
        this->domain_ptr = domain_ptr;

        memory.read16_vaddr(cpu, this->domain_ptr + DOMAIN_id, this->domain_id);

        memory.read8_vaddr (cpu, this->domain_ptr + DOMAIN_is_32bit_pv, this->is_32bit_pv);
        memory.read8_vaddr (cpu, this->domain_ptr + DOMAIN_is_hvm, this->is_hvm);
        memory.read8_vaddr (cpu, this->domain_ptr + DOMAIN_is_privileged, this->is_privileged);

        memory.read32_vaddr(cpu, this->domain_ptr + DOMAIN_max_vcpus, this->max_cpus);
        memory.read64_vaddr(cpu, this->domain_ptr + DOMAIN_vcpus, this->vcpus_ptr);

        memory.read32_vaddr(cpu, this->domain_ptr + DOMAIN_tot_pages, this->tot_pages);
        memory.read32_vaddr(cpu, this->domain_ptr + DOMAIN_max_pages, this->max_pages);
        memory.read32_vaddr(cpu, this->domain_ptr + DOMAIN_shr_pages, (uint32_t&)this->shr_pages);

        memory.read_block_vaddr(cpu, this->domain_ptr + DOMAIN_handle,
                                (char*)this->handle, sizeof this->handle);

        memory.read64_vaddr(cpu, this->domain_ptr + DOMAIN_next, this->next_domain_ptr);

        return true;
    }
    CATCH_COMMON

    return false;
}

bool x86_64Domain::parse_vcpus_basic(const CPU & cpu) throw ()
{
    try
    {
        host.validate_xen_vaddr(this->vcpus_ptr);

        if ( this->max_cpus < 1 )
        {
            LOG_ERROR("    No vcpus for domain\n");
            return false;
        }

        this->vcpus = new VCPU*[this->max_cpus];
        std::memset(this->vcpus, 0, sizeof (VCPU*) * this->max_cpus);

        LOG_INFO("    %"PRIu32" VCPUs\n", this->max_cpus);

        for ( uint32_t x = 0; x < this->max_cpus; ++x )
        {
            vaddr_t vcpu_addr;
            this->vcpus[x] = new x86_64VCPU();
            memory.read64_vaddr(cpu, this->vcpus_ptr + x * 8, vcpu_addr);
            host.validate_xen_vaddr(vcpu_addr);
            LOG_DEBUG("    Vcpu%"PRIu32" pointer = 0x%016"PRIx64"\n", x, vcpu_addr);
            this->vcpus[x]->parse_basic(vcpu_addr, cpu);
        }

        return true;
    }
    catch ( const std::bad_alloc & )
    {
        LOG_ERROR("Bad Alloc exception.  Out of memory\n");
    }
    CATCH_COMMON

    return false;
}


int x86_64Domain::print_state(FILE * o) const throw ()
{
    int len = 0;

    len += fprintf(o, "Domain %"PRIu16": (%d vcpus)\n", this->domain_id, this->max_cpus);

    len += fprintf(o, "  Flags:%s%s%s\n",
                   this->is_privileged ? " PRIVILEGED" : "",
                   this->is_32bit_pv ? " 32BIT-PV" : "",
                   this->is_hvm ? " HVM" : ""
        );
///@cond
#define PAGES_TO_GB(p) ((double)((p)<<12) / (1024.0 * 1024.0 * 1024.0))
#define PAGES_TO_MB(p) ((double)((p)<<12) / (1024.0 * 1024.0))
#define PAGES_TO_KB(p) ((double)((p)<<12) / (1024.0))

    len += fprintf(o, "  Max Pages: %"PRIu32" (%.3fGB, %.3fMB, %.fKB)\n",
                   this->max_pages, PAGES_TO_GB(this->max_pages),
                   PAGES_TO_MB(this->max_pages), PAGES_TO_KB(this->max_pages));
    len += fprintf(o, "  Current Pages: %"PRIu32"\n", this->tot_pages);
    len += fprintf(o, "  Shared Pages: %"PRId32"\n", this->shr_pages);

    len += fprintf(o, "  Handle: %02"PRIx8"%02"PRIx8"%02"PRIx8"%02"PRIx8"-%02"PRIx8
                   "%02"PRIx8"-%02"PRIx8"%02"PRIx8"-""%02"PRIx8"%02"PRIx8"-%02"PRIx8
                   "%02"PRIx8"%02"PRIx8"%02"PRIx8"%02"PRIx8"%02"PRIx8"\n",
                   this->handle[ 0], this->handle[ 1], this->handle[ 2], this->handle[ 3],
                   this->handle[ 4], this->handle[ 5], this->handle[ 6], this->handle[ 7],
                   this->handle[ 8], this->handle[ 9], this->handle[10], this->handle[11],
                   this->handle[12], this->handle[13], this->handle[14], this->handle[15] );


    len += fprintf(o, "\n");

    if ( this->domain_id == 0 )
        this->print_cmdline(o);

    for ( uint32_t x = 0; x < this->max_cpus; ++ x )
        if ( this->vcpus[x] )
        {
            len += fprintf(o, "  VCPU%"PRIu32":\n", this->vcpus[x]->vcpu_id);
            len += this->vcpus[x]->print_state(o);
        }
        else
            len += fprintf(o, "No information for vcpu%"PRIu32"\n", x);

    len += fprintf(o, "\n  Console Ring:\n");

    if ( this->domain_id == 0 )
        this->print_console(o);
    else
        len += fprintf(o, "    No Symbol Table\n");

#undef PAGES_TO_KB
#undef PAGES_TO_MB
#undef PAGES_TO_GB
/// @endcond
    return len;
}

int x86_64Domain::dump_structures(FILE * o) const throw ()
{
    const CPU & cpu = *static_cast<const CPU*>(this->vcpus[0]);
    int len = 0;

    if ( required_domain_symbols != 0 )
    {
        LOG_ERROR("Missing required domain symbols. %#x\n",
                  required_domain_symbols);
        return len;
    }

    len += fprintf(o, "Xen structures for Domain %"PRId16"\n\n", this->domain_id);

    len += fprintf(o, "struct domain (0x%016"PRIx64")\n", this->domain_ptr);
    len += dump_64bit_data(o, cpu, this->domain_ptr, DOMAIN_sizeof);

    for ( uint32_t x = 0; x < this->max_cpus; ++x )
        if ( this->vcpus[x] )
        {
            len += fprintf(o, "\n");
            len += this->vcpus[x]->dump_structures(o);
        }
        else
            len += fprintf(o, "Nothing to dump for vcpu%"PRIu32"\n\n", x);

    return len;
}

int x86_64Domain::print_console(FILE * o) const throw ()
{
    int len = 0;

    // Sanity check until we support multiple domain symbol tables
    if ( this->domain_id != 0 )
        return len;

    const CPU & cpu = *static_cast<const CPU*>(this->vcpus[0]);

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
        len += fprintf(o, "\tUnavailable, the following symbols are not available:\n");
        len += fprintf(o, "  %s%s%s.\n\n",
                       log_end_sym     == NULL ? " log_end"     : "",
                       log_buf_sym     == NULL ? " log_buf"     : "",
                       log_buf_len_sym == NULL ? " log_buf_len" : "");
        return len;
    }

    try
    {
        if ( this->is_32bit_pv )
        {
            memory.read32_vaddr(cpu, log_buf_sym->address, tmp);
            ring = tmp;
        }
        else
            memory.read64_vaddr(cpu, log_buf_sym->address, ring);

        memory.read32_vaddr(cpu, log_end_sym->address, tmp);
        producer = tmp;

        memory.read32_vaddr(cpu, log_buf_len_sym->address, tmp);
        length = tmp;


        if ( length > (1<<21) )
        {
            len += fprintf(o, "\tLength of 0x%"PRIx64" looks abnormally long.  Truncating to"
                           "0x%x.\n", length, 1<<16);
            length = 1<<16;
        }

        consumer = producer > length ? producer - length : 0;

        len += print_console_ring(o, cpu, ring, length, producer, consumer);
    }
    CATCH_COMMON

    return len;
}

int x86_64Domain::print_cmdline(FILE * o) const throw ()
{
    int len = 0;
    char * cmdline = NULL;

    // Sanity check until we support multiple domain symbol tables
    if ( this->domain_id != 0 )
        return len;

    const CPU & cpu = *static_cast<const CPU*>(this->vcpus[0]);

    const Symbol * cmdline_sym = host.dom0_symtab.find("saved_command_line");
    if ( ! cmdline_sym )
        len += fprintf(o, "Missing symbol for command line\n");
    else
    {
        try
        {
            // Size hardcoded in dom0
            cmdline = new char[2048];
            union { uint32_t val32; uint64_t val64; } cmdline_vaddr = {0};

            if ( this->is_32bit_pv )
                memory.read32_vaddr(cpu, cmdline_sym->address, cmdline_vaddr.val32);
            else
                memory.read64_vaddr(cpu, cmdline_sym->address, cmdline_vaddr.val64);

            memory.read_str_vaddr(cpu, cmdline_vaddr.val64, cmdline, 2047);
            len += fprintf(o, "  Command line: %s\n", cmdline);

            SAFE_DELETE_ARRAY(cmdline);
        }
        catch ( const std::bad_alloc & )
        {
            LOG_ERROR("Bad Alloc exception.  Out of memory\n");
        }
        CATCH_COMMON;
    }

    len += fprintf(o, "\n");
    SAFE_DELETE_ARRAY(cmdline);
    return len;
}

/*
 * Local variables:
 * mode: C++
 * c-set-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
