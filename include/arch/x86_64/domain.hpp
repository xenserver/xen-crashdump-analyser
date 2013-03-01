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

namespace x86_64
{

/**
 * Domain parser for x64 bit Xen.
 */
    class Domain : public Abstract::Domain
    {
    public:
        /**
         * Constructor.
         * @param xenpt Xen PageTables.
         */
        Domain(const Abstract::PageTable & xenpt);
        /// Destructor.
        virtual ~Domain();

        /**
         * Parse basic information from Xen's struct domain.
         *
         * @param domain_ptr Xen struct domain pointer.
         * @return boolean indicating success or failure.
         */
        virtual bool parse_basic(const vaddr_t & domain_ptr);

        /**
         * Parse basic VCPU information, based on domain information.
         *
         * @return boolean indicating success or failure.
         */
        virtual bool parse_vcpus_basic();

        /**
         * Print the information about this domain.
         * Information includes (where relevant).
         * - Memory info.
         * - Register state.
         *
         * @param stream Stream to write to.
         * @return Number of bytes written to stream.
         */
        virtual int print_state(FILE * stream) const;

        /**
         * Dump Xen structures for this domain.  Includes Xen's struct domain
         * and each struct vcpu.
         *
         * @param stream Stream to write to.
         * @return Number of bytes written to stream.
         */
        virtual int dump_structures(FILE * stream) const;

        /**
         * Print the console ring.
         *
         * @param stream Stream to write to.
         * @return Number of bytes written to stream.
         */
        virtual int print_console(FILE * stream) const;

        /**
         * Print the command line.
         *
         * @param stream Stream to write to.
         * @return Number of bytes written to stream.
         */
        virtual int print_cmdline(FILE * stream) const;

        /**
         * Get a usable set of Domain pagetables.
         * @throws Validate if no vcpus have suitable pagetables.
         * @returns Pagetables.
         */
        virtual const Abstract::PageTable & get_dompt() const;

    };

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
