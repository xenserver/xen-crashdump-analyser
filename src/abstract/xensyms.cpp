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
        VCPU_pause_flags, VCPU_pause_count;

    vaddr_t DOMAIN_sizeof, DOMAIN_id, DOMAIN_max_vcpus, DOMAIN_tot_pages,
        DOMAIN_max_pages, DOMAIN_shr_pages, DOMAIN_next, DOMAIN_is_hvm,
        DOMAIN_is_privileged, DOMAIN_vcpus, DOMAIN_pause_count, DOMAIN_handle;
    vaddr_t domain_list, idle_vcpu;

    vaddr_t VIRT_XEN_START, VIRT_XEN_END, VIRT_DIRECTMAP_START,
        VIRT_DIRECTMAP_END;

    vaddr_t XEN_DEBUG;

    vaddr_t LIST_HEAD_next;

    vaddr_t LIVEPATCH_payload_name,
            LIVEPATCH_payload_name_max_len,
            LIVEPATCH_payload_state,
            LIVEPATCH_payload_rc,
            LIVEPATCH_payload_buildid,
            LIVEPATCH_payload_buildid_len,
            LIVEPATCH_payload_text_addr,
            LIVEPATCH_payload_text_size,
            LIVEPATCH_payload_rw_addr,
            LIVEPATCH_payload_rw_size,
            LIVEPATCH_payload_ro_addr,
            LIVEPATCH_payload_ro_size,
            LIVEPATCH_payload_list,
            LIVEPATCH_payload_applied_list,
            LIVEPATCH_payload_symtab,
            LIVEPATCH_payload_nsyms,
            LIVEPATCH_symbol_name,
            LIVEPATCH_symbol_value,
            LIVEPATCH_symbol_sizeof,
            LIVEPATCH_symbol_max_len;

    /// @cond EXCLUDE
    DEFINE_XENSYM_GROUP(console);
    DEFINE_XENSYM_GROUP(consolepc);
    DEFINE_XENSYM_GROUP(vcpu);
    DEFINE_XENSYM_GROUP(domain);
    DEFINE_XENSYM_GROUP(misc);
    DEFINE_XENSYM_GROUP(virt);
    DEFINE_XENSYM_GROUP(livepatch);
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
        XENSYM(vcpu, VCPU_pause_count),

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
        XENSYM(domain, DOMAIN_pause_count),
        XENSYM(domain, DOMAIN_handle),
        XENSYM(domain, domain_list),
        XENSYM(domain, idle_vcpu),

        XENSYM(misc, XEN_DEBUG),
        XENSYM(misc, LIST_HEAD_next),

        XENSYM(virt, VIRT_XEN_START),
        XENSYM(virt, VIRT_XEN_END),
        XENSYM(virt, VIRT_DIRECTMAP_START),
        XENSYM(virt, VIRT_DIRECTMAP_END),

        XENSYM(livepatch, LIVEPATCH_payload_name),
        XENSYM(livepatch, LIVEPATCH_payload_name_max_len),
        XENSYM(livepatch, LIVEPATCH_payload_state),
        XENSYM(livepatch, LIVEPATCH_payload_rc),
        XENSYM(livepatch, LIVEPATCH_payload_buildid),
        XENSYM(livepatch, LIVEPATCH_payload_buildid_len),
        XENSYM(livepatch, LIVEPATCH_payload_text_addr),
        XENSYM(livepatch, LIVEPATCH_payload_text_size),
        XENSYM(livepatch, LIVEPATCH_payload_rw_addr),
        XENSYM(livepatch, LIVEPATCH_payload_rw_size),
        XENSYM(livepatch, LIVEPATCH_payload_ro_addr),
        XENSYM(livepatch, LIVEPATCH_payload_ro_size),
        XENSYM(livepatch, LIVEPATCH_payload_list),
        XENSYM(livepatch, LIVEPATCH_payload_applied_list),
        XENSYM(livepatch, LIVEPATCH_payload_symtab),
        XENSYM(livepatch, LIVEPATCH_payload_nsyms),
        XENSYM(livepatch, LIVEPATCH_symbol_name),
        XENSYM(livepatch, LIVEPATCH_symbol_value),
        XENSYM(livepatch, LIVEPATCH_symbol_sizeof),
        XENSYM(livepatch, LIVEPATCH_symbol_max_len),

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
