/*
 *  This file is part of the Xen Crashdump Analyser.
 *
 *  Xen Crashdump Analyser is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Xen Crashdump Analyser is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Xen Crashdump Analyser.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2011,2012 Citrix Inc.
 */

/**
 * @file src/arch/x86_64/vcpu.cpp
 * @author Andrew Cooper
 */

#include "arch/x86_64/vcpu.hpp"
#include "arch/x86_64/pagetable-walk.hpp"

#include <cstring>
#include <new>
#include "Xen.h"
#include "symbols.hpp"
#include "util/print-bitwise.hpp"
#include "host.hpp"
#include "memory.hpp"
#include "util/print-structures.hpp"
#include "util/log.hpp"
#include "util/macros.hpp"

x86_64VCPU::x86_64VCPU():
    regs()
{
    memset(&this->regs, 0, sizeof this->regs);
}

x86_64VCPU::~x86_64VCPU(){}

bool x86_64VCPU::parse_basic(const vaddr_t & addr, const CPU & cpu) throw ()
{
    if ( required_vcpu_symbols != 0 )
    {
        LOG_ERROR("Missing required vcpu symbols. %#x\n",
                  required_vcpu_symbols);
        return false;
    }
    if ( required_domain_symbols != 0 )
    {
        LOG_ERROR("Missing required domain symbols. %#x\n",
                  required_domain_symbols);
        return false;
    }

    try
    {
        host.validate_xen_vaddr(addr);
        this->vcpu_ptr = addr;

        memory.read64_vaddr(cpu, this->vcpu_ptr + VCPU_domain,
                            this->domain_ptr);

        host.validate_xen_vaddr(this->domain_ptr);

        memory.read32_vaddr(cpu, this->vcpu_ptr + VCPU_vcpu_id,
                            this->vcpu_id);

        memory.read32_vaddr(cpu, this->vcpu_ptr + VCPU_processor,
                            this->processor);

        memory.read16_vaddr(cpu, this->domain_ptr + DOMAIN_id,
                            this->domid);

        uint8_t is_32bit;
        memory.read8_vaddr(cpu, this->domain_ptr + DOMAIN_is_32bit_pv,
                           is_32bit);
        this->flags |= is_32bit ? CPU_PV_COMPAT : 0;

        uint32_t paging_mode;
        memory.read32_vaddr(cpu, this->domain_ptr + DOMAIN_paging_mode, paging_mode);
        if ( paging_mode == 0 )
            this->paging_support = VCPU::PAGING_NONE;
        else if ( paging_mode & (1U<<20) )
            this->paging_support = VCPU::PAGING_SHADOW;
        else if ( paging_mode & (1U<<21) )
            this->paging_support = VCPU::PAGING_HAP;

        memory.read32_vaddr(cpu, this->vcpu_ptr + VCPU_pause_flags,
                            this->pause_flags);

        memory.read64_vaddr(cpu, this->vcpu_ptr + VCPU_cr3,
                            this->regs.cr3);

        return true;
    }
    CATCH_COMMON

    return false;
}

bool x86_64VCPU::parse_regs(const vaddr_t & regs, const maddr_t & cr3) throw ()
{
    x86_64_cpu_user_regs * uregs = NULL;
    const CPU & cpu = *static_cast<const CPU*>(this);

    this->regs.cr3 = cr3;
    this->flags |= CPU_EXTD_STATE;

    try
    {
        host.validate_xen_vaddr(regs);
        uregs = new x86_64_cpu_user_regs();

        memory.read_block_vaddr(cpu, regs, (char*)uregs, sizeof *uregs );

        this->regs.r15 = uregs->r15;
        this->regs.r14 = uregs->r14;
        this->regs.r13 = uregs->r13;
        this->regs.r12 = uregs->r12;
        this->regs.rbp = uregs->rbp;
        this->regs.rbx = uregs->rbx;
        this->regs.r11 = uregs->r11;
        this->regs.r10 = uregs->r10;
        this->regs.r9 = uregs->r9;
        this->regs.r8 = uregs->r8;
        this->regs.rax = uregs->rax;
        this->regs.rcx = uregs->rcx;
        this->regs.rdx = uregs->rdx;
        this->regs.rsi = uregs->rsi;
        this->regs.rdi = uregs->rdi;
        this->regs.rip = uregs->rip;
        this->regs.cs = uregs->cs;
        this->regs.rflags = uregs->rflags;
        this->regs.rsp = uregs->rsp;
        this->regs.ds = uregs->ds;
        this->regs.es = uregs->es;
        this->regs.ss = uregs->ss;
        this->regs.fs = uregs->fs;
        this->regs.gs = uregs->gs;

        this->flags |= CPU_CORE_STATE;

        SAFE_DELETE(uregs);
        return true;
    }
    catch ( const std::bad_alloc & )
    {
        LOG_ERROR("Bad alloc of %"PRIu64" bytes for parsing vcpu structure "
                  "at 0x%016"PRIx64"\n", sizeof *uregs, this->vcpu_ptr);
    }
    CATCH_COMMON

    SAFE_DELETE(uregs);
    return false;
}

bool x86_64VCPU::parse_regs_from_struct() throw ()
{
    return this->parse_regs(this->vcpu_ptr + VCPU_user_regs, this->regs.cr3);
}

bool x86_64VCPU::parse_regs_from_stack(const vaddr_t & regs, const maddr_t & cr3) throw ()
{
    return this->parse_regs(regs, cr3);
}

bool x86_64VCPU::parse_regs_from_active(const VCPU* active) throw ()
{
    // Dangerous, but safe.  We will only actually be handed a 64bit vcpu;
    const x86_64VCPU * vcpu = reinterpret_cast<const x86_64VCPU *>(active);

    this->flags = vcpu->flags;
    this->regs = vcpu->regs;
    this->runstate = vcpu->runstate;
    return true;
}


void x86_64VCPU::pagetable_walk(const vaddr_t & vaddr, maddr_t & maddr, vaddr_t * page_end) const
{
    if ( ! this->flags & CPU_EXTD_STATE )
        throw pagefault(vaddr, 0ULL, 5);
    pagetable_walk_64(this->regs.cr3, vaddr, maddr, page_end);
}

bool x86_64VCPU::is_up() const throw () { return ! (this->pause_flags & 0x2); }

int x86_64VCPU::print_state(FILE * o) const throw ()
{
    int len = 0;

    if ( ! this->is_up() )
        return len + fprintf(o, "\tVCPU Offline\n\n");

    if ( this->flags & CPU_PV_COMPAT )
        return len + this->print_state_compat(o);

    if ( this->flags & CPU_CORE_STATE )
    {
        len += fprintf(o, "\tRIP:    %04x:[<%016"PRIx64">] Ring %d\n",
                       this->regs.cs, this->regs.rip, this->regs.cs & 0x3);
        len += fprintf(o, "\tRFLAGS: %016"PRIx64" ", this->regs.rflags);
        len += print_rflags(o, this->regs.rflags);
        len += fprintf(o, "\n\n");

        len += fprintf(o, "\trax: %016"PRIx64"   rbx: %016"PRIx64"   rcx: %016"PRIx64"\n",
                       this->regs.rax, this->regs.rbx, this->regs.rcx);
        len += fprintf(o, "\trdx: %016"PRIx64"   rsi: %016"PRIx64"   rdi: %016"PRIx64"\n",
                       this->regs.rdx, this->regs.rsi, this->regs.rdi);
        len += fprintf(o, "\trbp: %016"PRIx64"   rsp: %016"PRIx64"   r8:  %016"PRIx64"\n",
                       this->regs.rbp, this->regs.rsp, this->regs.r8);
        len += fprintf(o, "\tr9:  %016"PRIx64"   r10: %016"PRIx64"   r11: %016"PRIx64"\n",
                       this->regs.r9,  this->regs.r10, this->regs.r11);
        len += fprintf(o, "\tr12: %016"PRIx64"   r13: %016"PRIx64"   r14: %016"PRIx64"\n",
                       this->regs.r12, this->regs.r13, this->regs.r14);
        len += fprintf(o, "\tr15: %016"PRIx64"\n",
                       this->regs.r15);
    }

    if ( this->flags & CPU_EXTD_STATE )
    {
        len += fprintf(o, "\n");
        len += fprintf(o, "\tcr3: %016"PRIx64"\n", this->regs.cr3);
    }

    if ( this->flags & CPU_CORE_STATE )
    {
        len += fprintf(o, "\n");
        len += fprintf(o, "\tds: %04"PRIx16"   es: %04"PRIx16"   "
                       "fs: %04"PRIx16"   gs: %04"PRIx16"   "
                       "ss: %04"PRIx16"   cs: %04"PRIx16"\n",
                       this->regs.ds, this->regs.es, this->regs.fs,
                       this->regs.gs, this->regs.ss, this->regs.cs);
    }

    len += fprintf(o, "\n");

    len += fprintf(o, "\tPause Flags: 0x%"PRIx32" ", this->pause_flags);
    len += print_pause_flags(o, this->pause_flags);
    len += fprintf(o, "\n");

    switch ( this->runstate )
    {
    case RST_NONE:
        len += fprintf(o, "\tNot running:  Last run on PCPU%"PRIu32"\n", this->processor);
        break;
    case RST_RUNNING:
        len += fprintf(o, "\tCurrently running on PCPU%"PRIu32"\n", this->processor);
        break;
    case RST_CTX_SWITCH:
        len += fprintf(o, "\tBeing Context Switched:  State unreliable\n");
        break;
    default:
        len += fprintf(o, "\tUnknown runstate\n");
        break;
    }
    len += fprintf(o, "\tStruct vcpu at %016"PRIx64"\n", this->vcpu_ptr);

    len += fprintf(o, "\n");

    if ( this->flags & CPU_CORE_STATE &&
         this->flags & CPU_EXTD_STATE &&
         ( this->paging_support == VCPU::PAGING_NONE ||
           this->paging_support == VCPU::PAGING_SHADOW )
        )
    {
        len += fprintf(o, "\tStack at %16"PRIx64":", this->regs.rsp);
        len += print_64bit_stack(o, *static_cast<const CPU*>(this), this->regs.rsp);

        len += fprintf(o, "\n\tCode:\n");
        len += print_code(o, *static_cast<const CPU*>(this), this->regs.rip);

        len += fprintf(o, "\n\tCall Trace:\n");
        if ( this->domid == 0 )
        {
            vaddr_t sp = this->regs.rsp;
            vaddr_t top = (this->regs.rsp | (PAGE_SIZE-1))+1;
            uint64_t val;

            len += host.dom0_symtab.print_symbol64(o, this->regs.rip, true);

            try
            {
                while ( sp < top )
                {
                    memory.read64_vaddr(*static_cast<const CPU*>(this), sp, val);
                    len += host.dom0_symtab.print_symbol64(o, val);
                    sp += 8;
                }
            }
            CATCH_COMMON
        }
        else
            len += fprintf(o, "\t  No symbol table for domain\n");

        len += fprintf(o, "\n");
    }
    return len;
}

int x86_64VCPU::print_state_compat(FILE * o) const throw ()
{
    int len = 0;

    if ( this->flags & CPU_CORE_STATE )
    {
        len += fprintf(o, "\tEIP:    %04"PRIx16":[<%08"PRIx32">] Ring %d\n",
                       this->regs.cs, this->regs.eip, this->regs.cs & 0x3);
        len += fprintf(o, "\tEFLAGS: %08"PRIx32" ", this->regs.eflags);
        len += print_rflags(o, this->regs.rflags & -((uint32_t)1));
        len += fprintf(o, "\n");

        len += fprintf(o, "\teax: %08"PRIx32"   ebx: %08"PRIx32"   ",
                       this->regs.eax, this->regs.ebx);
        len += fprintf(o, "ecx: %08"PRIx32"   edx: %08"PRIx32"\n",
                       this->regs.ecx, this->regs.edx);
        len += fprintf(o, "\tesi: %08"PRIx32"   edi: %08"PRIx32"   ",
                       this->regs.esi, this->regs.edi);
        len += fprintf(o, "ebp: %08"PRIx32"   esp: %08"PRIx32"\n",
                       this->regs.ebp, this->regs.esp);
    }

    if ( this->flags & CPU_EXTD_STATE )
    {
        len += fprintf(o, "\tcr3: %016"PRIx64"\n", this->regs.cr3);
    }

    if ( this->flags & CPU_CORE_STATE )
    {
        len += fprintf(o, "\tds: %04"PRIx16"   es: %04"PRIx16"   "
                       "fs: %04"PRIx16"   gs: %04"PRIx16"   "
                       "ss: %04"PRIx16"   cs: %04"PRIx16"\n",
                       this->regs.ds, this->regs.es, this->regs.fs,
                       this->regs.gs, this->regs.ss, this->regs.cs);

    }

    len += fprintf(o, "\n");

    len += fprintf(o, "\tPause Flags: 0x%"PRIx32" ", this->pause_flags);
    len += print_pause_flags(o, this->pause_flags);
    len += fprintf(o, "\n");

    switch ( this->runstate )
    {
    case RST_NONE:
        len += fprintf(o, "\tNot running:  Last run on PCPU%"PRIu32"\n", this->processor);
        break;
    case RST_RUNNING:
        len += fprintf(o, "\tCurrently running on PCPU%"PRIu32"\n", this->processor);
        break;
    case RST_CTX_SWITCH:
        len += fprintf(o, "\tBeing Context Switched:  State unreliable\n");
        break;
    default:
        len += fprintf(o, "\tUnknown runstate\n");
        break;
    }
    len += fprintf(o, "\tStruct vcpu at %016"PRIx64"\n", this->vcpu_ptr);

    len += fprintf(o, "\n");

    if ( this->flags & CPU_CORE_STATE &&
        this->flags & CPU_EXTD_STATE )
    {
        len += fprintf(o, "\tStack at %08"PRIx32":", this->regs.esp);
        len += print_32bit_stack(o, *static_cast<const CPU*>(this), this->regs.rsp);

        len += fprintf(o, "\n\tCode:\n");
        len += print_code(o, *static_cast<const CPU*>(this), this->regs.rip);

        len += fprintf(o, "\n\tCall Trace:\n");
        if ( this->domid == 0 )
        {
            vaddr_t sp = this->regs.rsp;
            vaddr_t top = (this->regs.rsp | (PAGE_SIZE-1))+1;
            union { uint32_t val32; uint64_t val64; } val;
            val.val64 = 0;

            len += host.dom0_symtab.print_symbol32(o, this->regs.rip, true);

            try
            {
                while ( sp < top )
                {
                    memory.read32_vaddr(*static_cast<const CPU*>(this), sp, val.val32);
                    len += host.dom0_symtab.print_symbol32(o, val.val64);
                    sp += 4;
                }
            }
            CATCH_COMMON
        }
        else
            len += fprintf(o, "\t  No symbol table for domain\n");

    }

    len += fprintf(o, "\n");
    return len;
}

int x86_64VCPU::dump_structures(FILE * o) const throw ()
{
    const CPU & cpu = *static_cast<const CPU*>(this);
    int len = 0;

    if ( required_vcpu_symbols != 0 )
    {
        LOG_ERROR("Missing required domain symbols. %#x\n",
                  required_domain_symbols);
        return len;
    }

    len += fprintf(o, "struct vcpu (0x%016"PRIx64") for vcpu %"PRId32"\n",
                   this->vcpu_ptr, this->vcpu_id);
    len += dump_64bit_data(o, cpu, this->vcpu_ptr, VCPU_sizeof);
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
