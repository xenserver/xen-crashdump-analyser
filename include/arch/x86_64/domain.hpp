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

#ifndef __X86_64_DOMAIN_HPP__
#define __X86_64_DOMAIN_HPP__

/**
 * @file include/arch/x86_64/domain.hpp
 * @author Andrew Cooper
 */

#include "abstract/domain.hpp"

/**
 * Domain parser for x64 bit Xen.
 */
class x86_64Domain : public Domain
{
public:
    /// Constructor.
    x86_64Domain();
    /// Destructor.
    virtual ~x86_64Domain();

    /**
     * Parse basic information from Xen's struct domain.
     *
     * @param cpu CPU to perform pagetable lookups with.
     * @param domain_ptr Xen struct domain pointer.
     * @return boolean indicating success or failure.
     */
    virtual bool parse_basic(const CPU & cpu, const vaddr_t & domain_ptr) throw ();

    /**
     * Parse basic VCPU information, based on domain information.
     *
     * @param cpu CPU to perform pagetable lookups with.
     * @return boolean indicating success or failure.
     */
    virtual bool parse_vcpus_basic(const CPU & cpu) throw ();

    /**
     * Print the information about this domain.
     * Information includes (where relevant).
     * - Memory info.
     * - Register state.
     *
     * @param stream Stream to write to.
     * @return Number of bytes written to stream.
     */
    virtual int print_state(FILE * stream) const throw ();

    /**
     * Dump Xen structures for this domain.  Includes Xen's struct domain
     * and each struct vcpu.
     *
     * @param stream Stream to write to.
     * @return Number of bytes written to stream.
     */
    virtual int dump_structures(FILE * stream) const throw ();

    /**
     * Print the console ring.
     *
     * @param stream Stream to write to.
     * @return Number of bytes written to stream.
     */
    virtual int print_console(FILE * stream) const throw ();

    /**
     * Print the command line.
     *
     * @param stream Stream to write to.
     * @return Number of bytes written to stream.
     */
    virtual int print_cmdline(FILE * stream) const throw ();

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
