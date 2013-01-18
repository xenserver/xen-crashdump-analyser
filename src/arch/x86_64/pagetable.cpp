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
 *  Copyright (c) 2011,2012 Citrix Inc.
 */

/**
 * @file src/arch/x86_64/pagetable.cpp
 * @author Andrew Cooper
 */

#include "arch/x86_64/pagetable.hpp"
#include "arch/x86_64/pagetable-walk.hpp"

#include "exceptions.hpp"

namespace x86_64
{
    PT64::PT64(const uint64_t & cr3):cr3(cr3) {};
    PT64::~PT64() {};

    void PT64::walk(const vaddr_t & vaddr, maddr_t & maddr,
                       vaddr_t * page_end) const
    {
        /* Verify the pointer is canonical.  If not, the vaddr is
         * certainly junk. */
        if ( vaddr > 0x00007fffffffffffULL &&
             vaddr < 0xffff800000000000ULL )
            throw validate(vaddr, "Address is non-canonical.");

        pagetable_walk_64(this->cr3, vaddr, maddr, page_end);
    }


    PT64Compat::PT64Compat(const uint64_t & cr3):cr3(cr3) {};
    PT64Compat::~PT64Compat() {};

    void PT64Compat::walk(const vaddr_t & vaddr, maddr_t & maddr,
                       vaddr_t * page_end) const
    {
        /* Long compat mode uses 32bit pointers running on the same
         * 64bit pagetables, with a 0-extended pointer. */
        if ( vaddr & 0xffffffff00000000ULL )
            throw validate(vaddr, "Pointer out of range for 64bit Compat pagetables.");

        pagetable_walk_64(this->cr3, vaddr, maddr, page_end);
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
