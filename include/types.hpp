/*
 *  This file is part of the Xen Crashdump Analyser.
 *
 *  The Xen Crashdump Analyser is free software: you can redistribute
 *  it and/or modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation, either
 *  version 3 of the License, or (at your option) any later version.
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

#ifndef __TYPES_HPP__
#define __TYPES_HPP__

/**
 * @file types.hpp
 * @author Andrew Cooper
 */

#include <stdint.h>

/// Virtual address.  Should allow for 64bit pointers
typedef uint64_t vaddr_t;

/// Physical address.  Should allow for 64bit pointers
typedef uint64_t paddr_t;

/// Guest virtual address.
typedef vaddr_t gvaddr_t;
/// Guest physical address.
typedef vaddr_t gpaddr_t;
/// Machine address.
typedef paddr_t maddr_t;

/// Offset type.  Should allow for 64bits.
typedef unsigned long long offset_t;

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
