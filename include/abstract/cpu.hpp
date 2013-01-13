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

#ifndef __CPU_HPP__
#define __CPU_HPP__

/**
 * @file include/abstract/cpu.hpp
 * @author Andrew Cooper
 */

#include "types.hpp"
#include "exceptions.hpp"
#include <stddef.h>
#include <cstdio>

/**
 * Master abstract class for all types of CPUs
 */
class CPU
{
public:
    /// Constructor.
    CPU(){};
    /// Destructor.
    virtual ~CPU(){};

    /**
     * Is this CPU online?
     * @returns boolean
     */
    virtual bool is_online() const = 0;

    /**
     * Perform a pagetable lookup.
     * @param vaddr Virtual address to look up
     * @param maddr Machine address variable for the result.
     * @param page_end If non-null, variable to be filled with the last virtual address
     */
    virtual void pagetable_walk(const vaddr_t & vaddr, maddr_t & maddr,
                                vaddr_t * page_end = NULL) const = 0;
};

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
