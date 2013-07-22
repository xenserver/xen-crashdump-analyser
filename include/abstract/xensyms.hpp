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

#ifndef __XENSYMS_HPP__
#define __XENSYMS_HPP__

/**
 * @file include/abstract/xensyms.hpp
 * @author Andrew Cooper
 */

#include "util/xensym-common.hpp"

namespace Abstract
{
namespace xensyms
{
    /// Xen conring symbol, from symbol table.  Pointer to console ring.
    extern vaddr_t conring;
    /// Xen conring_size symbol, from symbol table.  Length of console ring.
    extern vaddr_t conring_size;

    /// Xen conringp symbol, from symbol table.  Console producer index.
    extern vaddr_t conringp;
    /// Xen conringc symbol, from symbol table  Console consumer index.
    extern vaddr_t conringc;

    /// Sizeof Xen's vcpu structure.
    extern vaddr_t VCPU_sizeof;
    /// Offset of vcpu_id in Xen's struct vcpu.
    extern vaddr_t VCPU_vcpu_id;
    /// Offset of processor in Xen's struct vcpu.
    extern vaddr_t VCPU_processor;
    /// Offset of domain in Xen's struct vcpu.
    extern vaddr_t VCPU_domain;
    /// Offset of pause_flags in Xen's struct vcpu.
    extern vaddr_t VCPU_pause_flags;

    /// Size of Xen's struct domain.
    extern vaddr_t DOMAIN_sizeof;
    /// Offset of id in Xen's struct domain.
    extern vaddr_t DOMAIN_id;
    /// Offset of max_vcpus in Xen's struct domain.
    extern vaddr_t DOMAIN_max_vcpus;
    /// Offset of tot_pages in Xen's struct domain.
    extern vaddr_t DOMAIN_tot_pages;
    /// Offset of max_pages in Xen's struct domain.
    extern vaddr_t DOMAIN_max_pages;
    /// Offset of shr_pages in Xen's struct domain.
    extern vaddr_t DOMAIN_shr_pages;
    /// Offset of next in Xen's struct domain.
    extern vaddr_t DOMAIN_next;
    /// Offset of is_hvm in Xen's struct domain.
    extern vaddr_t DOMAIN_is_hvm;
    /// Offset of is_privileged in Xen's struct domain.
    extern vaddr_t DOMAIN_is_privileged;
    /// Offset of vcpus in Xen's struct domain.
    extern vaddr_t DOMAIN_vcpus;
    /// Offset of handle in Xen's struct domain.
    extern vaddr_t DOMAIN_handle;
    /// Xen's domain_list symbol.
    extern vaddr_t domain_list;
    /// Xen's idle_vcpu symbol.
    extern vaddr_t idle_vcpu;

    /// Xen's code/data/bss start.
    extern vaddr_t VIRT_XEN_START;
    /// Xen's code/data/bss end.
    extern vaddr_t VIRT_XEN_END;
    /// Xen's 1to1 mapping of physical memory start.
    extern vaddr_t VIRT_DIRECTMAP_START;
    /// Xen's 1to1 mapping of physical memory end.
    extern vaddr_t VIRT_DIRECTMAP_END;

    /// In Xen a debug build?
    extern vaddr_t XEN_DEBUG;

    /// @cond EXCLUDE
    DECLARE_XENSYM_GROUP(console);
    DECLARE_XENSYM_GROUP(consolepc);
    DECLARE_XENSYM_GROUP(vcpu);
    DECLARE_XENSYM_GROUP(domain);
    DECLARE_XENSYM_GROUP(misc);
    DECLARE_XENSYM_GROUP(virt);
    /// @endcond

    /**
     * List of xensyms needed for parsing Xen memory, from architecture
     * independent areas of Xen.
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
#define REQ_CORE_XENSYMS(g) _required_xensyms(Abstract::xensyms::xensyms, \
                                              &Abstract::xensyms::_##g##_xsg_)
    /**
     * Check whether all group xensyms are present.
     *
     * @param g Xensym group name
     * @returns true if all symbols are present, or false if any are missing.
     */
#define HAVE_CORE_XENSYMS(g) (Abstract::xensyms::_##g##_xsg_ == 0ULL )

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
