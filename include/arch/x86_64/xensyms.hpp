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

#ifndef __X86_64_XENSYMS_HPP__
#define __X86_64_XENSYMS_HPP__

/**
 * @file include/arch/x86_64/xensyms.hpp
 * @author Andrew Cooper
 */

#include "util/xensym-common.hpp"

namespace x86_64
{
namespace xensyms
{
    /// Sizeof Xen's cpuinfo structure.
    extern vaddr_t CPUINFO_sizeof;
    /// Offset of processor_id in Xen's struct cpuinfo.
    extern vaddr_t CPUINFO_processor_id;
    /// Offset of current_vcpu in Xen's struct cpuinfo.
    extern vaddr_t CPUINFO_current_vcpu;
    /// Offset of per_cpu_offset in Xen's struct cpuinfo.
    extern vaddr_t CPUINFO_per_cpu_offset;
    /// Offset of guest_cpu_user_regs in Xen's struct cpuinfo.
    extern vaddr_t CPUINFO_guest_cpu_user_regs;

    /// Size of the kernel subset of Xen's struct cpu_user_regs.
    extern vaddr_t UREGS_kernel_sizeof;

    /// Offset of user_regs in Xen's struct arch_vcpu.
    extern vaddr_t VCPU_user_regs;
    /// Offset of flags in Xen's struct arch_vcpu.
    extern vaddr_t VCPU_flags;
    /// Offset of guest_table_user in Xen's struct arch_vcpu.
    extern vaddr_t VCPU_guest_table_user;
    /// Offset of guest_table in Xen's struct arch_vcpu.
    extern vaddr_t VCPU_guest_table;
    /// Offset of cr3 in Xen's struct arch_vcpu.
    extern vaddr_t VCPU_cr3;

    /// Offset of arch.paging.mode in Xen's struct arch_domain.
    extern vaddr_t DOMAIN_paging_mode;
    /// Offset of is_32bit_pv in Xen's struct arch_domain.
    extern vaddr_t DOMAIN_is_32bit_pv;

    /// Xen's per_cpu__curr_vcpu symbol.
    extern vaddr_t per_cpu__curr_vcpu;
    /// Xen's __per_cpu_offset symbol
    extern vaddr_t __per_cpu_offset;
    /// Xen's stack_base symbol.
    extern vaddr_t stack_base;

    /// @cond EXCLUDE
    DECLARE_XENSYM_GROUP(x86_64_cpuinfo);
    DECLARE_XENSYM_GROUP(x86_64_uregs);
    DECLARE_XENSYM_GROUP(x86_64_vcpu);
    DECLARE_XENSYM_GROUP(x86_64_domain);
    DECLARE_XENSYM_GROUP(x86_64_per_cpu);
    /// @endcond

    /**
     * List of xensyms needed for parsing Xen memory, from x86_64
     * architecture specific areas of Xen.
     */
    extern const struct xensym xensyms[];

    /**
     * Check whether all group xensyms are present.
     *
     * In the case that xensyms are missing, the xensym names are put in
     * an error message.
     * @param g Xensym group name
     * @returns true if all symbols are present, or false if any are missing.
     */
#define REQ_x86_64_XENSYMS(g) _required_xensyms(x86_64::xensyms::xensyms, \
                                                &x86_64::xensyms::_##g##_xsg_)

    /**
     * Check whether all group xensyms are present.
     *
     * @param g Xensym group name
     * @returns true if all symbols are present, or false if any are missing.
     */
#define HAVE_x86_64_XENSYMS(g) (x86_64::xensyms::_##g##_xsg_ == 0ULL )

}
}

#endif

/*
 * Local variables:
 * mode: C++
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
