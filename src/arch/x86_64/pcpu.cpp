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
 * @file src/arch/x86_64/pcpu.cpp
 * @author Andrew Cooper
 */

#include "arch/x86_64/pcpu.hpp"
#include "arch/x86_64/vcpu.hpp"
#include "arch/x86_64/structures.hpp"
#include "arch/x86_64/pagetable.hpp"
#include "arch/x86_64/xensyms.hpp"

#include "Xen.h"

#include "abstract/xensyms.hpp"

#include "host.hpp"
#include "util/print-bitwise.hpp"
#include "util/print-structures.hpp"
#include "util/log.hpp"
#include "util/macros.hpp"
#include "util/stdio-wrapper.hpp"
#include "util/misc.hpp"
#include "memory.hpp"

#include <new>

using namespace Abstract::xensyms;
using namespace x86_64::xensyms;


namespace x86_64
{

    PCPU::PCPU():
        regs()
    {
        memset(&this->regs, 0, sizeof this->regs );
    }

    PCPU::~PCPU()
    {
        SAFE_DELETE(this->ctx_from);
        SAFE_DELETE(this->ctx_to);
        SAFE_DELETE(this->vcpu);
    }

    bool PCPU::parse_pr_status(const char * buff, const size_t len, int index)
    {
        ELF_Prstatus * ptr = (ELF_Prstatus *)buff;

        if ( len != sizeof *ptr )
        {
            this->online = false;
            LOG_WARN("Wrong size for pr_status note %d.  Expected %zu, got %zu\n",
                     index, sizeof *ptr, len);
            return false;
        }

        if ( is_zeroes(buff, len) )
        {
            this->online = false;
            LOG_WARN("Got zeros for pr_status note %d - PCPU assumed down\n", index);
            return false;
        }

        this->regs.r15 = ptr->pr_reg[PR_REG_r15];
        this->regs.r14 = ptr->pr_reg[PR_REG_r14];
        this->regs.r13 = ptr->pr_reg[PR_REG_r13];
        this->regs.r12 = ptr->pr_reg[PR_REG_r12];
        this->regs.rbp = ptr->pr_reg[PR_REG_rbp];
        this->regs.rbx = ptr->pr_reg[PR_REG_rbx];
        this->regs.r11 = ptr->pr_reg[PR_REG_r11];
        this->regs.r10 = ptr->pr_reg[PR_REG_r10];
        this->regs.r9 = ptr->pr_reg[PR_REG_r9];
        this->regs.r8 = ptr->pr_reg[PR_REG_r8];
        this->regs.rax = ptr->pr_reg[PR_REG_rax];
        this->regs.rcx = ptr->pr_reg[PR_REG_rcx];
        this->regs.rdx = ptr->pr_reg[PR_REG_rdx];
        this->regs.rsi = ptr->pr_reg[PR_REG_rsi];
        this->regs.rdi = ptr->pr_reg[PR_REG_rdi];
        this->regs.orig_rax = ptr->pr_reg[PR_REG_orig_rax];
        this->regs.rip = ptr->pr_reg[PR_REG_rip];
        this->regs.cs = ptr->pr_reg[PR_REG_cs];
        this->regs.rflags = ptr->pr_reg[PR_REG_rflags];
        this->regs.rsp = ptr->pr_reg[PR_REG_rsp];
        this->regs.ds = ptr->pr_reg[PR_REG_ds];
        this->regs.es = ptr->pr_reg[PR_REG_es];
        this->regs.ss = ptr->pr_reg[PR_REG_ss];
        this->regs.fs = ptr->pr_reg[PR_REG_fs];
        this->regs.gs = ptr->pr_reg[PR_REG_gs];

        this->flags |= ( CPU_GP_REGS | CPU_SEG_REGS );
        return true;
    }

    bool PCPU::parse_xen_crash_core(const char * buff, const size_t len, int index)
    {
        x86_64_crash_xen_core_t * ptr = (x86_64_crash_xen_core_t*)buff;

        if ( len != sizeof *ptr )
        {
            this->online = false;
            LOG_WARN("Wrong size for crash_xen_core note %d.  Expected %zu, got %zu\n",
                     index, sizeof *ptr, len);
            return false;
        }

        if ( is_zeroes(buff, len) )
        {
            this->online = false;
            LOG_WARN("Got zeros for xen_crash_core note %d - PCPU assumed down\n", index);
            return false;
        }

        this->regs.cr0 = ptr->cr0;
        this->regs.cr2 = ptr->cr2;
        this->regs.cr3 = ptr->cr3;
        this->regs.cr4 = ptr->cr4;

        if ( this->regs.cr3 == 0ULL )
        {
            this->online = false;
            LOG_WARN("Got cr3 of 0 from xen_crash_core note %d - PCPU assumed down\n", index);
            return false;
        }

        try
        {
            this->xenpt = new PT64(this->regs.cr3);
        }
        catch ( const std::bad_alloc & )
        {
            LOG_ERROR("Bad alloc for PCPU vcpus.  Kdump environment needs more memory\n");
            return false;
        }

        this->flags |= CPU_CR_REGS;

        return true;
    }

    bool PCPU::decode_extended_state()
    {
        if ( ! this->online )
        {
            LOG_ERROR("  This PCPU is not online\n");
            return false;
        }
        if ( ! (this->flags & CPU_CR_REGS) )
        {
            LOG_ERROR("  Missing required CPU_CR_REGS for this pcpu\n");
            return false;
        }
        if ( ! ( REQ_x86_64_XENSYMS(x86_64_cpuinfo) &
                 REQ_x86_64_XENSYMS(x86_64_per_cpu) &
                 REQ_x86_64_XENSYMS(x86_64_uregs) ))
            return false;

        try
        {
            vaddr_t cpu_info = this->regs.rsp;
            cpu_info &= ~(STACK_SIZE-1);
            cpu_info |= STACK_SIZE - CPUINFO_sizeof;

            host.validate_xen_vaddr(cpu_info);

            uint32_t pid;
            memory.read32_vaddr(*this->xenpt, cpu_info + CPUINFO_processor_id,
                                pid);
            this->processor_id = pid;

            LOG_INFO("  Processor ID %u\n", this->processor_id);

            if ( this->processor_id > host.nr_pcpus )
            {
                LOG_ERROR("  Processor id exceeds the host cpu number\n");
                return false;
            }

            memory.read64_vaddr(*this->xenpt, cpu_info + CPUINFO_current_vcpu,
                                this->current_vcpu_ptr);
            host.validate_xen_vaddr(this->current_vcpu_ptr);


            memory.read64_vaddr(*this->xenpt, cpu_info + CPUINFO_per_cpu_offset,
                                this->per_cpu_offset);
            memory.read64_vaddr(*this->xenpt, this->per_cpu_offset + per_cpu__curr_vcpu,
                                this->per_cpu_current_vcpu_ptr);

            host.validate_xen_vaddr(this->per_cpu_current_vcpu_ptr);

            LOG_DEBUG("    Current vcpu 0x%016"PRIx64"%s, per-cpu vcpu 0x%016"PRIx64
                      "%s (per-cpu offset 0x%016"PRIx64")\n",
                      this->current_vcpu_ptr,
                      this->current_vcpu_ptr == host.idle_vcpus[this->processor_id]
                      ? " (IDLE)" : "",
                      this->per_cpu_current_vcpu_ptr,
                      this->per_cpu_current_vcpu_ptr == host.idle_vcpus[this->processor_id]
                      ? " (IDLE)" : "",
                      this->per_cpu_offset);

            if ( this->per_cpu_current_vcpu_ptr == host.idle_vcpus[this->processor_id] )
            {
                LOG_INFO("    PCPU has no associated VCPU.\n");
                this->vcpu_state = CTX_NONE;
            }
            else if ( this->current_vcpu_ptr == host.idle_vcpus[this->processor_id] )
            {
                LOG_INFO("    Current vcpu is IDLE.  Guest context on stack.\n");
                this->vcpu_state = CTX_IDLE;
                // Load this->vcpu from per_cpu_current_vcpu_ptr, regs from stack
                this->vcpu = new VCPU(Abstract::VCPU::RST_NONE);
                if ( ! this->vcpu->parse_basic(
                         this->per_cpu_current_vcpu_ptr, *this->xenpt) ||
                     ! this->vcpu->parse_extended(*this->xenpt, &cpu_info) )
                    return false;
            }
            else
            {
                if ( this->current_vcpu_ptr == this->per_cpu_current_vcpu_ptr )
                {
                    LOG_INFO("    Current vcpu was RUNNING.  Guest context on stack\n");
                    this->vcpu_state = CTX_RUNNING;
                    // Load this->vcpu from per_cpu_current_vcpu_ptr, regs on stack
                    this->vcpu = new VCPU(Abstract::VCPU::RST_RUNNING);

                    /* If Xen is currently on an IST stack, consider an alternate location
                     * for the guest GP registers. */
                    if ( STACK_PAGE(this->regs.rsp) < 3 )
                    {
                        x86_64exception exp_regs;
                        uint64_t stack_top = (this->regs.rsp | (PAGE_SIZE-1))+1 - sizeof exp_regs;
                        memory.read_block_vaddr(*this->xenpt, stack_top,
                                                (char*)&exp_regs, sizeof exp_regs);

                        /* If we interrupted a PV guest, its GP state is here rather than
                         * on the main stack. */
                        if ( (exp_regs.cs & 0x3) != 0 )
                        {
                            LOG_INFO("      Running on IST with guest context at top\n");
                            cpu_info = (this->regs.rsp | (PAGE_SIZE-1))+1 - UREGS_kernel_sizeof;
                        }
                    }

                    if ( ! this->vcpu->parse_basic(
                             this->per_cpu_current_vcpu_ptr, *this->xenpt) ||
                         ! this->vcpu->parse_extended(*this->xenpt, &cpu_info) )
                        return false;
                }
                else
                {
                    LOG_INFO("    Xen was context switching.  Guest context inaccurate\n");
                    /* Context switch was occurring.  ctx_from has indeterminate register
                     * state.  ctx_to can find valid register state in its struct vcpu.
                     */
                    this->vcpu_state = CTX_SWITCH;
                    this->ctx_from = new VCPU(Abstract::VCPU::RST_CTX_SWITCH);
                    if ( ! this->ctx_from->parse_basic(
                             this->per_cpu_current_vcpu_ptr, *this->xenpt) ||
                         ! this->ctx_from->parse_extended(*this->xenpt, &cpu_info) )
                        return false;

                    this->ctx_to = new VCPU(Abstract::VCPU::RST_NONE);
                    if ( ! this->ctx_to->parse_basic(
                             this->current_vcpu_ptr, *this->xenpt) )
                        return false;
                }
            }

            this->flags |= CPU_STACK_STATE;
            return true;
        }
        catch ( const std::bad_alloc & )
        {
            LOG_ERROR("Bad alloc for PCPU vcpus.  Kdump environment needs more memory\n");
        }
        catch ( const CommonError & e )
        {
            e.log();
        }

        return false;
    }

    bool PCPU::is_online() const { return this->online; }

    int PCPU::print_state(FILE * o) const
    {
        int len = 0;
        Abstract::VCPU * vcpu_to_print = NULL;

        len += FPRINTF(o, "  PCPU %d Host state:\n", this->processor_id);

        if ( !this->online )
        {
            return len + FPUTS("    PCPU Offline\n\n", o);
        }

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
            len += FPUTS("\n", o);

            len += FPRINTF(o, "\tcr0: %016"PRIx64"  ", this->regs.cr0);
            len += print_cr0(o, this->regs.cr0);
            len += FPUTS("\n", o);

            len += FPRINTF(o, "\tcr3: %016"PRIx64"   cr2: %016"PRIx64"\n",
                           this->regs.cr3, this->regs.cr2);

            len += FPRINTF(o, "\tcr4: %016"PRIx64"  ", this->regs.cr4);
            len += print_cr4(o, this->regs.cr4);
            len += FPUTS("\n", o);
        }

        if ( this->flags & CPU_GP_REGS )
        {
            len += FPUTS("\n", o);
            len += FPRINTF(o, "\tds: %04"PRIx16"   es: %04"PRIx16"   "
                           "fs: %04"PRIx16"   gs: %04"PRIx16"   "
                           "ss: %04"PRIx16"   cs: %04"PRIx16"\n",
                           this->regs.ds, this->regs.es, this->regs.fs,
                           this->regs.gs, this->regs.ss, this->regs.cs);
        }

        len += FPUTS("\n", o);

        if ( this->flags & CPU_STACK_STATE )
        {
            switch ( this->vcpu_state )
            {
            case CTX_NONE:
                len += FPRINTF(o, "\tpercpu current VCPU %016"PRIx64" IDLE\n",
                               this->per_cpu_current_vcpu_ptr);
                len += FPUTS("\tNo associated VCPU\n", o);
                break;

            case CTX_IDLE:
                len += FPRINTF(o, "\tstack current VCPU  %016"PRIx64" IDLE\n",
                               this->current_vcpu_ptr);
                len += FPRINTF(o, "\tpercpu current VCPU %016"PRIx64" DOM%"PRIu16" VCPU%"PRIu32"\n",
                               this->per_cpu_current_vcpu_ptr, this->vcpu->domid, this->vcpu->vcpu_id);
                len += FPUTS("\tVCPU was IDLE\n", o);
                break;

            case CTX_RUNNING:
                len += FPRINTF(o, "\tstack current VCPU  %016"PRIx64" DOM%"PRIu16" VCPU%"PRIu32"\n",
                               this->current_vcpu_ptr, this->vcpu->domid, this->vcpu->vcpu_id);
                len += FPRINTF(o, "\tpercpu current VCPU %016"PRIx64" DOM%"PRIu16" VCPU%"PRIu32"\n",
                               this->per_cpu_current_vcpu_ptr, this->vcpu->domid, this->vcpu->vcpu_id);
                len += FPUTS("\tVCPU was RUNNING\n", o);
                vcpu_to_print = this->vcpu;
                break;

            case CTX_SWITCH:
                len += FPRINTF(o, "\tstack current VCPU  %016"PRIx64" DOM%"PRIu16" VCPU%"PRIu32"\n",
                               this->current_vcpu_ptr, this->ctx_from->domid, this->ctx_from->vcpu_id);
                len += FPRINTF(o, "\tpercpu current VCPU %016"PRIx64" DOM%"PRIu16" VCPU%"PRIu32"\n",
                               this->per_cpu_current_vcpu_ptr, this->ctx_to->domid,
                               this->ctx_to->vcpu_id);
                len += FPRINTF(o, "\tXen was context switching from DOM%"PRIu16" VCPU%"
                               PRIu32" to DOM%"PRIu16" VCPU%"PRIu32"\n",
                               this->ctx_from->domid, this->ctx_from->vcpu_id,
                               this->ctx_to->domid, this->ctx_to->vcpu_id );
                vcpu_to_print = this->ctx_from;
                break;

            case CTX_UNKNOWN:
            default:
                len += FPUTS("\tUnable to parse stack information\n", o);
                break;
            }
        }

        len += FPUTS("\n", o);

        len += FPRINTF(o, "\tStack at %016"PRIx64":", this->regs.rsp);
        len += print_64bit_stack(o, *this->xenpt, this->regs.rsp);

        len += FPUTS("\n\tCode:\n", o);
        len += print_code(o, *this->xenpt, this->regs.rip);

        len += FPUTS("\n\tCall Trace:\n", o);

        uint64_t val = this->regs.rip;
        len += host.symtab.print_symbol64(o, val, true);

        this->print_stack(o, this->regs.rsp, 0);

        len += FPUTS("\n", o);

        if ( vcpu_to_print )
        {
            len += FPRINTF(o, "  PCPU %"PRIu32" Guest state (DOM%"PRIu16" VCPU%"PRIu32"):\n",
                           vcpu_to_print->processor, vcpu_to_print->domid, vcpu_to_print->vcpu_id);
            len += vcpu_to_print->print_state(o);
        }

        return len;
    }

    int PCPU::dump_stack(FILE * o) const
    {
        static const char * stack_name[] = { "Double Fault", "NMI", "MCE", "Normal" };

        vaddr_t stack_min = this->regs.rsp & ~(STACK_SIZE-1);
        vaddr_t stack_max = stack_min | (STACK_SIZE-1);

        int len = 0;

        try
        {
            len += FPRINTF(o, "PCPU %d\n", this->processor_id);
            len += FPRINTF(o, "  rsp 0x%016"PRIx64", min 0x%016"PRIx64", max 0x%016"PRIx64"\n\n",
                           this->regs.rsp, stack_min, stack_max);

            if ( !host.validate_xen_vaddr(stack_min, false) ||
                 !host.validate_xen_vaddr(stack_max, false) )
            {
                len += FPRINTF(o, "Failed to validate stack ends.  Giving up.\n");
                return len;
            }

            for ( int stack_page = 0; stack_page < 8; ++stack_page )
            {
                vaddr_t page_base = stack_min + stack_page * PAGE_SIZE;
                vaddr_t page_max  = page_base | (PAGE_SIZE-1);

                maddr_t frame;

                len += FPRINTF(o, "Stack page %d, 0x%016"PRIx64"-0x%016"PRIx64" (%s stack)\n",
                               stack_page, page_base, page_max, stack_name[std::min(stack_page,3)]);
                try
                {
                    this->xenpt->walk(page_base, frame, NULL);
                }
                catch ( const pagefault & e )
                {
                    if ( e.level == 1 && e.reason == pagefault::FAULT_NOTPRESENT)
                    {
                        len += FPUTS("  Not present (Guard page?)\n\n", o);
                        continue;
                    }
                    throw;
                }

                len += FPUTS("\n", o);

                uint64_t val;
                uint8_t zero_mask = 0x3f, zeroes = zero_mask;
                bool printed_something = false;

                for ( vaddr_t sp = page_base ; sp < page_max ; sp += 8, frame += 8 )
                {
                    memory.read64(frame, val);

                    if ( zeroes == zero_mask )
                    {
                        if ( val == 0 )
                            continue;
                        else if ( sp != page_base )
                            len += FPRINTF(o, "Truncating block of zeroes\n");
                    }
                    zeroes = (zeroes << 1 | !val) & zero_mask;

                    len += FPRINTF(o, "  %016"PRIx64": %016"PRIx64, sp, val);

                    if ( val >= stack_min && val <= stack_max )
                        len += FPRINTF(o, " .%+d\n", (int)(val - sp));
                    else if ( host.symtab.is_text_symbol(val) )
                    {
                        len += FPUTS(" ", o);
                        len += host.symtab.print_text_symbol(o, val);
                        len += FPUTS("\n", o);
                    }
                    else
                        len += FPUTS("\n", o);

                    printed_something = true;
                }

                if ( !printed_something )
                    len += FPUTS("Page was entirely zeroes\n", o);
                else if ( zeroes == zero_mask )
                    len += FPUTS("Truncating range of zeroes\n", o);

                len += FPUTS("\n", o);
            }
        }
        catch ( const CommonError & e )
        {
            e.log();
        }

        return len;
    }


    int PCPU::print_stack(FILE * o, const vaddr_t & stack, unsigned mask) const
    {
        static const char * stack_name[] = { "Double Fault", "NMI", "MCE", "Normal" };
        uint64_t sp = stack;
        int len = 0;

        // Stack frames 3 thru 7 form the normal Xen stack.  Stacks 0 thru 2 are special
        const unsigned stack_page = STACK_PAGE(sp) < 3 ? STACK_PAGE(sp) : 3;

        try
        {
            uint64_t stack_top, val;
            x86_64exception exp_regs;

            host.validate_xen_vaddr(stack);

            if ( mask & (1U << stack_page) )
            {
                // Bail - we have already visited this stack
                len += FPRINTF(o, "\t  Not recursing.  Already visited the %s stack "
                               "(%u, mask %#x)\n", stack_name[stack_page],
                               stack_page, mask);
                return len;
            }
            else
                // Mark this stack_page as having been visited
                mask |= (1U << stack_page);

            if ( stack_page <= 2 )
                // Entered this stack frame from NMI, MCE or Double Fault
                stack_top = (sp | (PAGE_SIZE-1))+1 - sizeof exp_regs;
            else
            {
                stack_top = this->regs.rsp;
                stack_top &= ~(STACK_SIZE-1);
                stack_top |= STACK_SIZE - CPUINFO_sizeof;
            }

            while ( sp < stack_top )
            {
                memory.read64_vaddr(*this->xenpt, sp, val);
                len += host.symtab.print_symbol64(o, val);
                sp += 8;
            }

            if ( stack_page <= 2 )
            {
                // This hardware interrupt interrupted something else, most likely Xen
                memory.read_block_vaddr(*this->xenpt, stack_top, (char*)&exp_regs, sizeof exp_regs);

                len += FPRINTF(o, "\n\t      %s interrupted Code at %04"PRIx16":%016"PRIx64
                               " and Stack at %04"PRIx16":%016"PRIx64"\n\n",
                               stack_name[stack_page], exp_regs.cs,
                               exp_regs.rip, exp_regs.ss, exp_regs.rsp);

                // Did we interrupt non-ring0 context? Perhaps we interrupted the VCPU
                if ( (exp_regs.cs & 3) != 0 )
                    return len + FPUTS("\t  Interrupted VCPU context\n", o);

                if ( (stack_top & ~(STACK_SIZE-1)) != (exp_regs.rsp & ~(STACK_SIZE-1)) )
                {
                    LOG_WARN("Exception frame rsp (0x%016"PRIx64") moves off current stack "
                             "(0x%016"PRIx64") - Not following\n", exp_regs.rsp, stack_top);
                    return len;
                }

                len += host.symtab.print_symbol64(o, exp_regs.rip, true);
                len += this->print_stack(o, exp_regs.rsp, mask);
            }
        }
        catch ( const CommonError & e )
        {
            e.log();
        }

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
