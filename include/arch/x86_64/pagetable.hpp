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

#ifndef __X86_64_PAGETABLE_HPP__
#define __X86_64_PAGETABLE_HPP__

/**
 * @file include/arch/x86_64/pagetable.hpp
 * @author Andrew Cooper
 */

#include "abstract/pagetable.hpp"

namespace x86_64
{
    /**
     * Basic 64bit Pagetable abstraction.
     */
    class PT64: public Abstract::PageTable
    {
    public:
        /**
         * Constructor.
         * @param cr3 Control Register 3
         */
        PT64(const uint64_t & cr3);
        /// Destructor.
        virtual ~PT64();

        /**
         * Perform a pagetable walk.
         * @param vaddr Virtual address to look up.
         * @param maddr Machine address variable for the result.
         * @param page_end If non-null, variable to be filled with the
         * last virtual address of the page.
         */
        virtual void walk(const vaddr_t & vaddr, maddr_t & maddr,
                          vaddr_t * page_end = NULL) const;

        /**
         * Retrieve the root of this set of pagetables.
         * @returns cr3 equivalent for this set of pagetables.
         */
        virtual uint64_t root() const;

    private:
        /// Control Register 3
        uint64_t cr3;
    };

    /**
     * Basic 32bit Pagetable abstraction
     *
     * This is functionally equivelent to PT64 as Long mode and
     * Compat mode use the same paging structure, but will verify
     * that the virtual address is 32 bits wide.
     */
    class PT64Compat: public Abstract::PageTable
    {
    public:
        /**
         * Constructor.
         * @param cr3 Control Register 3
         */
        PT64Compat(const uint64_t & cr3);
        /// Destructor.
        virtual ~PT64Compat();

        /**
         * Perform a pagetable walk.
         * @param vaddr Virtual address to look up.
         * @param maddr Machine address variable for the result.
         * @param page_end If non-null, variable to be filled with the
         * last virtual address of the page.
         */
        virtual void walk(const vaddr_t & vaddr, maddr_t & maddr,
                          vaddr_t * page_end = NULL) const;

        /**
         * Retrieve the root of this set of pagetables.
         * @returns cr3 equivalent for this set of pagetables.
         */
        virtual uint64_t root() const;

    private:
        /// Control Register 3
        uint64_t cr3;
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
