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
 * @file include/arch/x86_64/pagetable-walk.hpp
 * @author Andrew Cooper
 */

#include "types.hpp"
#include "exceptions.hpp"

/**
 * Pagetable walk for 64bit mode.
 * Long mode executing 64bit code has a 52bit physical address space and a 64bit
 * virtual address space.
 * @param cr3 Value of the cr3 register.
 * @param vaddr Virtual address to look up.
 * @param maddr Machine address result of the pagetable walk.
 * @param page_end If non-null, variable to be filled with the last virtual address
 * within the page which contains vaddr.
 * @throws memseek
 * @throws memread
 * @throws pagefault
 */
void pagetable_walk_64(const maddr_t & cr3, const vaddr_t & vaddr,
                       maddr_t & maddr, vaddr_t * page_end = NULL);

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
