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

#ifndef __DOMAIN_HPP__
#define __DOMAIN_HPP__

/**
 * @file include/abstract/domain.hpp
 * @author Andrew Cooper
 */

#include <stddef.h>
#include <cstdio>

#include "abstract/vcpu.hpp"

/**
 * Domain parser
 */
class Domain
{
public:
    /// Constructor.
    Domain():
        domain_ptr(0), next_domain_ptr(0), domain_id(0), is_32bit_pv(0), is_hvm(0),
        is_privileged(0), tot_pages(0), max_pages(0), shr_pages(0), max_cpus(0),
        vcpus_ptr(0), paging_mode(0), vcpus(NULL)
    {};

    /// Destructor.
    virtual ~Domain() {};

    /**
     * Parse basic information from Xen's struct domain.
     *
     * @param cpu CPU to perform pagetable lookups with.
     * @param domain_ptr Xen struct domain pointer.
     * @return boolean indicating success or failure.
     */
    virtual bool parse_basic(const CPU & cpu, const vaddr_t & domain_ptr) throw() = 0;

    /**
     * Parse basic VCPU information, based on domain information.
     *
     * @param cpu CPU to perform pagetable lookups with.
     * @return boolean indicating success or failure.
     */
    virtual bool parse_vcpus_basic(const CPU & cpu) throw () = 0;

    /**
     * Print the information about this domain.
     * Information includes (where relevant).
     * - Memory info.
     * - Register state.
     *
     * @param stream Stream to write to.
     * @return Number of bytes written to stream.
     */
    virtual int print_state(FILE * stream) const throw () = 0;

    /**
     * Dump Xen structures for this domain.  Includes Xen's struct domain
     * and each struct vcpu.
     *
     * @param stream Stream to write to.
     * @return Number of bytes written to stream.
     */
    virtual int dump_structures(FILE * stream) const throw () = 0;

    /**
     * Print the console ring.
     *
     * @param stream Stream to write to.
     * @return Number of bytes written to stream.
     */
    virtual int print_console(FILE * stream) const throw () = 0;

    /**
     * Print the command line.
     *
     * @param stream Stream to write to.
     * @return Number of bytes written to stream.
     */
    virtual int print_cmdline(FILE * stream) const throw () = 0;

    /// Xen struct domain pointer.
    vaddr_t domain_ptr;
    /// Pointer to next domain.
    vaddr_t next_domain_ptr;
    /// dom id.
    uint16_t domain_id;
    /// Whether this domain is 32 bit PV.
    uint8_t is_32bit_pv,
    /// Whether this domain is using HVM extensions.
        is_hvm,
    /// Whether this domain is privileged.
        is_privileged;
    /// Total pages.
    uint32_t tot_pages,
    /// Maximum pages.
        max_pages;
    /// Shared pages
    int32_t shr_pages;
    /// Maximum VCPUs.
    uint32_t max_cpus;
    /// Pointer to VCPU* array.
    vaddr_t vcpus_ptr;
    /// Handle (Toolstack domain reference).
    uint8_t handle[16];
    /// Paging mode flags
    uint32_t paging_mode;

    /// VCPUs for this domain.
    VCPU ** vcpus;

private:
    // @cond
    Domain(const Domain & );
    Domain & operator= (const Domain &);
    // @endcond
};

#endif

/*
 * Local variables:
 * mode: C++
 * c-set-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
