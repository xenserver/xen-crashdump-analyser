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
 * @file src/arch/x86_64/vcpu.cpp
 * @author Andrew Cooper
 */

#include "arch/x86_64/vcpu.hpp"
#include "arch/x86_64/pagetable.hpp"
#include "arch/x86_64/xensyms.hpp"

#include <cstring>
#include <new>
#include "Xen.h"
#include "abstract/xensyms.hpp"
#include "util/print-bitwise.hpp"
#include "host.hpp"
#include "memory.hpp"
#include "util/print-structures.hpp"
#include "util/log.hpp"
#include "util/macros.hpp"
#include "util/stdio-wrapper.hpp"

using namespace Abstract::xensyms;
using namespace x86_64::xensyms;

namespace x86_64
{

    VCPU::VCPU(Abstract::VCPU::VCPURunstate rst):
        Abstract::VCPU(rst), arch_flags(0), guest_table_user(0),
        guest_table(0), regs()
    {
        memset(&this->regs, 0, sizeof this->regs);
    }

    VCPU::~VCPU(){}

    bool VCPU::parse_basic(const vaddr_t & addr, const Abstract::PageTable & xenpt)
    {
        if ( ! ( REQ_CORE_XENSYMS(domain) &
                 REQ_CORE_XENSYMS(vcpu) &
                 REQ_x86_64_XENSYMS(x86_64_domain) &
                 REQ_x86_64_XENSYMS(x86_64_vcpu) ))
            return false;

        try
        {
            host.validate_xen_vaddr(addr);
            this->vcpu_ptr = addr;

            memory.read64_vaddr(xenpt, this->vcpu_ptr + VCPU_domain,
                                this->domain_ptr);

            host.validate_xen_vaddr(this->domain_ptr);

            memory.read32_vaddr(xenpt, this->vcpu_ptr + VCPU_vcpu_id,
                                this->vcpu_id);

            memory.read32_vaddr(xenpt, this->vcpu_ptr + VCPU_processor,
                                this->processor);

            memory.read16_vaddr(xenpt, this->domain_ptr + DOMAIN_id,
                                this->domid);

            uint8_t is_32bit;
            memory.read8_vaddr(xenpt, this->domain_ptr + DOMAIN_is_32bit_pv,
                               is_32bit);
            this->flags |= is_32bit ? CPU_PV_COMPAT : 0;

            uint32_t paging_mode;
            memory.read32_vaddr(xenpt, this->domain_ptr + DOMAIN_paging_mode, paging_mode);
            if ( paging_mode == 0 )
                this->paging_support = VCPU::PAGING_NONE;
            else if ( paging_mode & (1U<<20) )
                this->paging_support = VCPU::PAGING_SHADOW;
            else if ( paging_mode & (1U<<21) )
                this->paging_support = VCPU::PAGING_HAP;

            memory.read32_vaddr(xenpt, this->vcpu_ptr + VCPU_pause_flags,
                                this->pause_flags);
            memory.read32_vaddr(xenpt, this->vcpu_ptr + VCPU_pause_count,
                                this->pause_count);

            memory.read64_vaddr(xenpt, this->vcpu_ptr + VCPU_flags,
                                this->arch_flags);
            memory.read64_vaddr(xenpt, this->vcpu_ptr + VCPU_guest_table_user,
                                this->guest_table_user);
            this->guest_table_user = this->guest_table_user << PAGE_SHIFT;
            memory.read64_vaddr(xenpt, this->vcpu_ptr + VCPU_guest_table,
                                this->guest_table);
            this->guest_table = this->guest_table << PAGE_SHIFT;
            memory.read64_vaddr(xenpt, this->vcpu_ptr + VCPU_cr3,
                                this->regs.cr3);

            this->flags |= CPU_CR_REGS;

            return true;
        }
        catch ( const CommonError & e )
        {
            e.log();
        }

        return false;
    }

    bool VCPU::parse_extended(const Abstract::PageTable & xenpt,
                              const vaddr_t * cpuinfo)
    {
        try
        {
            if ( this->guest_table == 0ULL )
            {
                LOG_WARN("Cannot get kernel page table address - VCPU assumed down\n");
                return false;
            }

            if ( this->flags & CPU_PV_COMPAT )
                this->dompt = new x86_64::PT64Compat(this->guest_table);
            else
                this->dompt = new x86_64::PT64(this->guest_table);

            switch ( this->runstate )
            {
            case RST_NONE:
                this->parse_gp_regs(this->vcpu_ptr + VCPU_user_regs, xenpt);
                this->parse_seg_regs(this->vcpu_ptr + VCPU_user_regs, xenpt);
                break;

            case RST_RUNNING:
            case RST_CTX_SWITCH:
                if ( ! cpuinfo )
                {
                    LOG_ERROR("Needed Xen per-pcpu stack cpuinfo to parse "
                              "d%"PRId16"v%"PRId32", but got NULL\n",
                              this->domid, this->vcpu_id);
                    return false;
                }

                this->parse_gp_regs(*cpuinfo + CPUINFO_guest_cpu_user_regs, xenpt);
                this->parse_seg_regs(this->vcpu_ptr + VCPU_user_regs, xenpt);
                break;

            case RST_UNKNOWN:
                LOG_ERROR("Bad vcpu runstate for parsing extended state\n");
                return false;
            }

            return true;
        }
        catch ( const std::bad_alloc & )
        {
            LOG_ERROR("Bad alloc - out of memory\n");
        }
        catch ( const CommonError & e )
        {
            e.log();
        }

        return false;
    }

    bool VCPU::parse_gp_regs(const vaddr_t & regs, const Abstract::PageTable & xenpt)
    {
        x86_64_cpu_user_regs * uregs = NULL;

        try
        {
            host.validate_xen_vaddr(regs);
            uregs = new x86_64_cpu_user_regs();

            memory.read_block_vaddr(xenpt, regs, (char*)uregs, sizeof *uregs );

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
            this->regs.ss = uregs->ss;

            this->flags |= CPU_GP_REGS;

            SAFE_DELETE(uregs);
            return true;
        }
        catch ( const std::bad_alloc & )
        {
            LOG_ERROR("Bad alloc of %"PRIu64" bytes for parsing vcpu structure "
                      "at 0x%016"PRIx64"\n", sizeof *uregs, this->vcpu_ptr);
        }
        catch ( const CommonError & e )
        {
            e.log();
        }

        SAFE_DELETE(uregs);
        return false;
    }

    bool VCPU::parse_seg_regs(const vaddr_t & regs, const Abstract::PageTable & xenpt)
    {
        x86_64_cpu_user_regs * uregs = NULL;

        try
        {
            host.validate_xen_vaddr(regs);
            uregs = new x86_64_cpu_user_regs();

            memory.read_block_vaddr(xenpt, regs, (char*)uregs, sizeof *uregs );

            this->regs.ds = uregs->ds;
            this->regs.es = uregs->es;
            this->regs.fs = uregs->fs;
            this->regs.gs = uregs->gs;

            this->flags |= CPU_SEG_REGS;

            SAFE_DELETE(uregs);
            return true;
        }
        catch ( const std::bad_alloc & )
        {
            LOG_ERROR("Bad alloc of %"PRIu64" bytes for parsing vcpu structure "
                      "at 0x%016"PRIx64"\n", sizeof *uregs, this->vcpu_ptr);
        }
        catch ( const CommonError & e )
        {
            e.log();
        }

        SAFE_DELETE(uregs);
        return false;
    }

    bool VCPU::copy_from_active(const Abstract::VCPU* active)
    {
        // Dangerous, but safe.  We will only actually be handed a 64bit vcpu;
        const VCPU * vcpu = reinterpret_cast<const VCPU *>(active);

        try
        {
            if ( vcpu->guest_table == 0ULL )
            {
                LOG_ERROR("Cannot get kernel page table address from active VCPU\n");
                return false;
            }

            if ( this->flags & CPU_PV_COMPAT )
                this->dompt = new x86_64::PT64Compat(vcpu->guest_table);
            else
                this->dompt = new x86_64::PT64(vcpu->guest_table);
        }
        catch ( const std::bad_alloc & )
        {
            LOG_ERROR("Bad Alloc exception.  Out of memory\n");
            return false;
        }
        catch ( const CommonError & e )
        {
            e.log();
            return false;
        }

        this->flags = vcpu->flags;
        this->regs = vcpu->regs;
        this->runstate = vcpu->runstate;
        this->arch_flags = vcpu->arch_flags;
        this->guest_table_user = vcpu->guest_table_user;
        this->guest_table = vcpu->guest_table;
        return true;
    }


    bool VCPU::is_online() const { return ! (this->pause_flags & 0x2); }

    int VCPU::print_state(FILE * o) const
    {
        int len = 0;

        if ( ! this->is_online() )
            return len + FPUTS("\tVCPU Offline\n\n", o);

        if ( this->flags & CPU_PV_COMPAT )
            return len + this->print_state_compat(o);

        if ( this->flags & CPU_GP_REGS )
        {
            len += FPRINTF(o, "\tRIP:    %04x:[<%016"PRIx64">] Ring %d\n",
                           this->regs.cs, this->regs.rip, this->regs.cs & 0x3);
            len += FPRINTF(o, "\tRFLAGS: %016"PRIx64" ", this->regs.rflags);
            len += print_rflags(o, this->regs.rflags);
            len += FPUTS("\n\n", o);

            len += FPRINTF(o, "\trax: %016"PRIx64"   rbx: %016"PRIx64"   rcx: %016"PRIx64"\n",
                           this->regs.rax, this->regs.rbx, this->regs.rcx);
            len += FPRINTF(o, "\trdx: %016"PRIx64"   rsi: %016"PRIx64"   rdi: %016"PRIx64"\n",
                           this->regs.rdx, this->regs.rsi, this->regs.rdi);
            len += FPRINTF(o, "\trbp: %016"PRIx64"   rsp: %016"PRIx64"   r8:  %016"PRIx64"\n",
                           this->regs.rbp, this->regs.rsp, this->regs.r8);
            len += FPRINTF(o, "\tr9:  %016"PRIx64"   r10: %016"PRIx64"   r11: %016"PRIx64"\n",
                           this->regs.r9,  this->regs.r10, this->regs.r11);
            len += FPRINTF(o, "\tr12: %016"PRIx64"   r13: %016"PRIx64"   r14: %016"PRIx64"\n",
                           this->regs.r12, this->regs.r13, this->regs.r14);
            len += FPRINTF(o, "\tr15: %016"PRIx64"\n",
                           this->regs.r15);
        }

        if ( this->flags & CPU_CR_REGS )
        {
            len += FPRINTF(o, "\n\tguest_table_user: %016"PRIx64"\n",
                           this->guest_table_user);
            len += FPRINTF(o, "\tguest_table: %016"PRIx64"\n",
                           this->guest_table);
            len += FPRINTF(o, "\tHW cr3: %016"PRIx64"\n", this->regs.cr3);
        }

        if ( (this->flags & CPU_CR_REGS) &&
             (this->flags & CPU_SEG_REGS) )
        {
            len += FPUTS("\n", o);

            if ( this->flags & CPU_SEG_REGS )
                len += FPRINTF(o, "\tds: %04"PRIx16"   es: %04"PRIx16"   "
                               "fs: %04"PRIx16"   gs: %04"PRIx16"   "
                               "ss: %04"PRIx16"   cs: %04"PRIx16"\n",
                               this->regs.ds, this->regs.es, this->regs.fs,
                               this->regs.gs, this->regs.ss, this->regs.cs);
            else
                len += FPRINTF(o, "\tss: %04"PRIx16"   cs: %04"PRIx16"\n",
                               this->regs.ss, this->regs.cs);
        }

        len += FPUTS("\n", o);

        len += FPRINTF(o, "\tPause Count: %"PRId32", Flags: 0x%"PRIx32" ",
                       this->pause_count, this->pause_flags);
        len += print_pause_flags(o, this->pause_flags);
        len += FPUTS("\n", o);

        switch ( this->runstate )
        {
        case RST_NONE:
            len += FPRINTF(o, "\tNot running:  Last run on PCPU%"PRIu32"\n", this->processor);
            break;
        case RST_RUNNING:
            len += FPRINTF(o, "\tCurrently running on PCPU%"PRIu32"\n", this->processor);
            break;
        case RST_CTX_SWITCH:
            len += FPUTS("\tBeing Context Switched:  State unreliable\n", o);
            break;
        case RST_UNKNOWN:
            len += FPUTS("\tUnknown runstate\n", o);
            break;
        }
        len += FPRINTF(o, "\tStruct vcpu at %016"PRIx64"\n", this->vcpu_ptr);
        len += FPRINTF(o, "\tVCPU in %s mode\n",
                       this->arch_flags & TF_kernel_mode ? "kernel" : "user");

        len += FPUTS("\n", o);

        if ( this->flags & CPU_GP_REGS &&
             this->flags & CPU_CR_REGS &&
             this->arch_flags & TF_kernel_mode &&
             ( this->paging_support == VCPU::PAGING_NONE ||
               this->paging_support == VCPU::PAGING_SHADOW )
            )
        {
            len += FPRINTF(o, "\tStack at %16"PRIx64":", this->regs.rsp);
            len += print_64bit_stack(o, *this->dompt, this->regs.rsp);

            len += FPUTS("\n\tCode:\n", o);
            len += print_code(o, *this->dompt, this->regs.rip);

            len += FPUTS("\n\tCall Trace:\n", o);
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
                        memory.read64_vaddr(*this->dompt, sp, val);
                        len += host.dom0_symtab.print_symbol64(o, val);
                        sp += 8;
                    }
                }
                catch ( const CommonError & e )
                {
                    e.log();
                }
            }
            else
                len += FPUTS("\t  No symbol table for domain\n", o);

            len += FPUTS("\n", o);
        }
        return len;
    }

    int VCPU::print_state_compat(FILE * o) const
    {
        int len = 0;

        if ( this->flags & CPU_GP_REGS )
        {
            len += FPRINTF(o, "\tEIP:    %04"PRIx16":[<%08"PRIx32">] Ring %d\n",
                           this->regs.cs, this->regs.eip, this->regs.cs & 0x3);
            len += FPRINTF(o, "\tEFLAGS: %08"PRIx32" ", this->regs.eflags);
            len += print_rflags(o, this->regs.rflags & -((uint32_t)1));
            len += FPUTS("\n", o);

            len += FPRINTF(o, "\teax: %08"PRIx32"   ebx: %08"PRIx32"   ",
                           this->regs.eax, this->regs.ebx);
            len += FPRINTF(o, "ecx: %08"PRIx32"   edx: %08"PRIx32"\n",
                           this->regs.ecx, this->regs.edx);
            len += FPRINTF(o, "\tesi: %08"PRIx32"   edi: %08"PRIx32"   ",
                           this->regs.esi, this->regs.edi);
            len += FPRINTF(o, "ebp: %08"PRIx32"   esp: %08"PRIx32"\n",
                           this->regs.ebp, this->regs.esp);
        }

        if ( this->flags & CPU_CR_REGS )
        {
            len += FPRINTF(o, "\n\tguest_table_user: %016"PRIx64"\n",
                           this->guest_table_user);
            len += FPRINTF(o, "\tguest_table: %016"PRIx64"\n",
                           this->guest_table);
            len += FPRINTF(o, "\tHW cr3: %016"PRIx64"\n", this->regs.cr3);
        }

        if ( (this->flags & CPU_CR_REGS) &&
             (this->flags & CPU_SEG_REGS) )
        {
            len += FPUTS("\n", o);

            if ( this->flags & CPU_SEG_REGS )
                len += FPRINTF(o, "\tds: %04"PRIx16"   es: %04"PRIx16"   "
                               "fs: %04"PRIx16"   gs: %04"PRIx16"   "
                               "ss: %04"PRIx16"   cs: %04"PRIx16"\n",
                               this->regs.ds, this->regs.es, this->regs.fs,
                               this->regs.gs, this->regs.ss, this->regs.cs);
            else
                len += FPRINTF(o, "\tss: %04"PRIx16"   cs: %04"PRIx16"\n",
                               this->regs.ss, this->regs.cs);
        }

        len += FPUTS("\n", o);

        len += FPRINTF(o, "\tPause Count: %"PRId32", Flags: 0x%"PRIx32" ",
                       this->pause_count, this->pause_flags);
        len += print_pause_flags(o, this->pause_flags);
        len += FPUTS("\n", o);

        switch ( this->runstate )
        {
        case RST_NONE:
            len += FPRINTF(o, "\tNot running:  Last run on PCPU%"PRIu32"\n", this->processor);
            break;
        case RST_RUNNING:
            len += FPRINTF(o, "\tCurrently running on PCPU%"PRIu32"\n", this->processor);
            break;
        case RST_CTX_SWITCH:
            len += FPUTS("\tBeing Context Switched:  State unreliable\n", o);
            break;
        default:
            len += FPUTS("\tUnknown runstate\n", o);
            break;
        }
        len += FPRINTF(o, "\tStruct vcpu at %016"PRIx64"\n", this->vcpu_ptr);
        len += FPRINTF(o, "\tVCPU in %s mode\n",
                       this->arch_flags & TF_kernel_mode ? "kernel" : "user");

        len += FPUTS("\n", o);

        if ( this->flags & CPU_GP_REGS &&
             this->flags & CPU_CR_REGS &&
             this->arch_flags & TF_kernel_mode)
        {
            len += FPRINTF(o, "\tStack at %08"PRIx32":", this->regs.esp);
            len += print_32bit_stack(o, *this->dompt, this->regs.rsp);

            len += FPUTS("\n\tCode:\n", o);
            len += print_code(o, *this->dompt, this->regs.rip);

            len += FPUTS("\n\tCall Trace:\n", o);
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
                        memory.read32_vaddr(*this->dompt, sp, val.val32);
                        len += host.dom0_symtab.print_symbol32(o, val.val64);
                        sp += 4;
                    }
                }
                catch ( const CommonError & e )
                {
                    e.log();
                }
            }
            else
                len += FPUTS("\t  No symbol table for domain\n", o);

        }

        len += FPUTS("\n", o);
        return len;
    }

    int VCPU::dump_structures(FILE * o, const Abstract::PageTable & xenpt) const
    {
        int len = 0;

        if ( ! ( REQ_CORE_XENSYMS(vcpu) ))
            return len;

        len += FPRINTF(o, "struct vcpu (0x%016"PRIx64") for vcpu %"PRId32"\n",
                       this->vcpu_ptr, this->vcpu_id);
        len += dump_64bit_data(o, xenpt, this->vcpu_ptr, VCPU_sizeof);
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
