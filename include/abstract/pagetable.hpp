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

#ifndef __PAGETABLE_HPP__
#define __PAGETABLE_HPP__

/**
 * @file include/abstract/pagetable.hpp
 * @author Andrew Cooper
 */

#include <cstring>
#include "types.hpp"

namespace Abstract
{
    /**
     * Abstract base class for all pagetable walking operations.
     *
     * Xen has a very complicated set of paging operations, depending on
     * architectures, type of domain, mode of VCPUs, and current
     * toolstack operations such as migration.
     *
     * This class acts as an interface for all common code to be able to
     * perform pagetable lookups in the context of a pcpu or vcpu.
     */
    class PageTable
    {
    public:
        /// Constructor.
        PageTable() {};
        /// Destructor.
        virtual ~PageTable() {};

        /**
         * Perform a pagetable walk.
         * @param vaddr Virtual address to look up.
         * @param maddr Machine address variable for the result.
         * @param page_end If non-null, variable to be filled with the
         * last virtual address of the page.
         */
        virtual void walk(const vaddr_t & vaddr, maddr_t & maddr,
                          vaddr_t * page_end = NULL) const = 0;

        /**
         * Retrieve the root of this set of pagetables.
         * @returns cr3 equivalent for this set of pagetables.
         */
        virtual uint64_t root() const = 0;
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
