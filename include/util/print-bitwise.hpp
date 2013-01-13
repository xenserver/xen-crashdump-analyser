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

/**
 * @file include/util/print-bitwise.hpp
 * @author Andrew Cooper
 */

#include "types.hpp"
#include <cstdio>

/**
 * Bitwise decode cr0 to stream.
 *
 * @param stream Stream to print to.
 * @param cr0 CR0 register to decode.
 * @return number of bytes written.
 */
int print_cr0(FILE * stream, const uint64_t & cr0);

/**
 * Bitwise decode cr4 to stream.
 *
 * @param stream Stream to print to.
 * @param cr4 CR4 register to decode.
 * @return number of bytes written.
 */
int print_cr4(FILE * stream, const uint64_t & cr4);

/**
 * Bitwise decode rflags to stream.
 *
 * @param stream Stream to print to.
 * @param rflags register to decode.
 * @return number of bytes written.
 */
int print_rflags(FILE * stream, const uint64_t & rflags);

/**
 * Bitwise decode a vcpu's pause_flags to stream.
 *
 * @param stream Stream to print to.
 * @param pause_flags to decode.
 * @return number of bytes written.
 */
int print_pause_flags(FILE * stream, const uint32_t & pause_flags);

/**
 * Bitwise decode a domains paging mode assistance flags to stream.
 *
 * @param stream Stream to print to.
 * @param paging_mode to decode.
 * @return number of bytes written.
 */
int print_paging_mode(FILE * stream, const uint32_t & paging_mode);

/*
 * Local variables:
 * mode: C++
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
