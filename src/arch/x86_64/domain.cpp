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
#include "util/print-bitwise.hpp"
#include "util/log.hpp"
#include "util/macros.hpp"
#include "util/symbol.hpp"
#include "util/stdio-wrapper.hpp"

/**
 * @file src/arch/x86_64/domain.cpp
 * @author Andrew Cooper
 */

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
                this->vcpus[x] = new VCPU();
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


    int Domain::print_state(FILE * o) const
    {
        int len = 0;

        len += FPRINTF(o, "Domain %"PRIu16": (%d vcpus)\n", this->domain_id, this->max_cpus);

        len += FPRINTF(o, "  Flags:%s%s%s\n",
                       this->is_privileged ? " PRIVILEGED" : "",
                       this->is_32bit_pv ? " 32BIT-PV" : "",
                       this->is_hvm ? " HVM" : ""
            );

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

        if ( this->domain_id == 0 )
            this->print_cmdline(o);

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
            this->print_console(o);
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

        if ( required_domain_symbols != 0 )
        {
            LOG_ERROR("Missing required domain symbols. %#x\n",
                      required_domain_symbols);
            return len;
        }

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

    int Domain::print_console(FILE * o) const
    {
        int len = 0;

        // Sanity check until we support multiple domain symbol tables
        if ( this->domain_id != 0 )
            return len;

        const Abstract::PageTable & dompt = *this->vcpus[0]->dompt;

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
            len += FPUTS("\tUnavailable, the following symbols are not available:\n", o);
            len += FPRINTF(o, "  %s%s%s.\n\n",
                           log_end_sym     == NULL ? " log_end"     : "",
                           log_buf_sym     == NULL ? " log_buf"     : "",
                           log_buf_len_sym == NULL ? " log_buf_len" : "");
            return len;
        }

        try
        {
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

    int Domain::print_cmdline(FILE * o) const
    {
        int len = 0;
        char * cmdline = NULL;

        // Sanity check until we support multiple domain symbol tables
        if ( this->domain_id != 0 )
            return len;

        const Abstract::PageTable & dompt = *this->vcpus[0]->dompt;

        const Symbol * cmdline_sym = host.dom0_symtab.find("saved_command_line");
        if ( ! cmdline_sym )
            len += FPUTS("Missing symbol for command line\n", o);
        else
        {
            try
            {
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
