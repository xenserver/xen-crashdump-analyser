/*
 *  This file is part of the Xen Crashdump Analyser.
 *
 *  Foobar is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Foobar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2012 Citrix Inc.
 */

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

/**
 * @file include/system.hpp
 * @author Andrew Cooper
 */

#include "types.hpp"

/// CPU Vendors.
enum CPU_VENDOR
{
    /// Unknown vendor.
    VENDOR_UNKNOWN,
    /// Intel.
    VENDOR_INTEL,
    /// AMD.
    VENDOR_AMD
};

/// The cpu vendor, from cpuid.
extern enum CPU_VENDOR cpu_vendor;

/// Number of bits of the maximum physical address.
extern uint8_t maxphysaddr;
/// Mask generated from maxphysaddr.
extern uint64_t physaddrmask;

/**
 * Gather information about the CPUs needed for effective decoding of
 * Xen structures.  Information gets stored in the above global variables.
 */
void gather_system_information();

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
