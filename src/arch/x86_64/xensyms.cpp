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

/**
 * @file src/arch/x86_64/xensyms.cpp
 * @author Andrew Cooper
 */

#include "arch/x86_64/xensyms.hpp"

namespace x86_64
{
namespace xensyms
{
    vaddr_t CPUINFO_sizeof, CPUINFO_processor_id, CPUINFO_current_vcpu,
        CPUINFO_per_cpu_offset, CPUINFO_guest_cpu_user_regs;

    vaddr_t UREGS_kernel_sizeof;

    vaddr_t VCPU_user_regs, VCPU_flags, VCPU_guest_table_user,
        VCPU_guest_table, VCPU_cr3;

    vaddr_t DOMAIN_paging_mode, DOMAIN_is_32bit_pv;

    vaddr_t per_cpu__curr_vcpu, __per_cpu_offset;

    /// @cond EXCLUDE
    DEFINE_XENSYM_GROUP(x86_64_cpuinfo);
    DEFINE_XENSYM_GROUP(x86_64_uregs);
    DEFINE_XENSYM_GROUP(x86_64_vcpu);
    DEFINE_XENSYM_GROUP(x86_64_domain);
    DEFINE_XENSYM_GROUP(x86_64_per_cpu);
    /// @endcond

    const struct xensym xensyms [] =
    {
        XENSYM(x86_64_cpuinfo, CPUINFO_sizeof),
        XENSYM(x86_64_cpuinfo, CPUINFO_processor_id),
        XENSYM(x86_64_cpuinfo, CPUINFO_current_vcpu),
        XENSYM(x86_64_cpuinfo, CPUINFO_per_cpu_offset),
        XENSYM(x86_64_cpuinfo, CPUINFO_guest_cpu_user_regs),

        XENSYM(x86_64_uregs, UREGS_kernel_sizeof),

        XENSYM(x86_64_vcpu, VCPU_user_regs),
        XENSYM(x86_64_vcpu, VCPU_flags),
        XENSYM(x86_64_vcpu, VCPU_guest_table_user),
        XENSYM(x86_64_vcpu, VCPU_guest_table),
        XENSYM(x86_64_vcpu, VCPU_cr3),

        XENSYM(x86_64_domain, DOMAIN_paging_mode),
        XENSYM(x86_64_domain, DOMAIN_is_32bit_pv),

        XENSYM(x86_64_per_cpu, per_cpu__curr_vcpu),
        XENSYM(x86_64_per_cpu, __per_cpu_offset),

        XENSYM_NULL
    };
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
