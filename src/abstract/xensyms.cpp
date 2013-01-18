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
 * @file src/abstract/xensyms.cpp
 * @author Andrew Cooper
 */

#include "abstract/xensyms.hpp"

namespace Abstract
{
namespace xensyms
{

    vaddr_t conring, conring_size;

    vaddr_t conringp, conringc;

    vaddr_t VCPU_sizeof, VCPU_vcpu_id, VCPU_processor, VCPU_domain,
        VCPU_pause_flags;

    vaddr_t DOMAIN_sizeof, DOMAIN_id, DOMAIN_max_vcpus, DOMAIN_tot_pages,
        DOMAIN_max_pages, DOMAIN_shr_pages, DOMAIN_next, DOMAIN_is_hvm,
        DOMAIN_is_privileged, DOMAIN_vcpus, DOMAIN_handle;
    vaddr_t domain_list, idle_vcpu;

    /// @cond EXCLUDE
    DEFINE_XENSYM_GROUP(console);
    DEFINE_XENSYM_GROUP(consolepc);
    DEFINE_XENSYM_GROUP(vcpu);
    DEFINE_XENSYM_GROUP(domain);
    /// @endcond

    const struct xensym xensyms [] =
    {
        XENSYM(console, conring),
        XENSYM(console, conring_size),

        XENSYM(consolepc, conringp),
        XENSYM(consolepc, conringc),

        XENSYM(vcpu, VCPU_sizeof),
        XENSYM(vcpu, VCPU_vcpu_id),
        XENSYM(vcpu, VCPU_processor),
        XENSYM(vcpu, VCPU_domain),
        XENSYM(vcpu, VCPU_pause_flags),

        XENSYM(domain, DOMAIN_sizeof),
        XENSYM(domain, DOMAIN_id),
        XENSYM(domain, DOMAIN_max_vcpus),
        XENSYM(domain, DOMAIN_tot_pages),
        XENSYM(domain, DOMAIN_max_pages),
        XENSYM(domain, DOMAIN_shr_pages),
        XENSYM(domain, DOMAIN_next),
        XENSYM(domain, DOMAIN_is_hvm),
        XENSYM(domain, DOMAIN_is_privileged),
        XENSYM(domain, DOMAIN_vcpus),
        XENSYM(domain, DOMAIN_handle),
        XENSYM(domain, domain_list),
        XENSYM(domain, idle_vcpu),

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
