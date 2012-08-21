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
 * @file include/util/print-structures.hpp
 * @author Andrew Cooper
 */

#include "types.hpp"
#include "abstract/cpu.hpp"
#include <cstdio>

/**
 * Print a 64bit stack dump.
 * @param stream Stream to print to.
 * @param cpu CPU to do a pagetable lookup with.
 * @param rsp Stack pointer to start at.
 * @param count Number of entries to print.  Defaults to rounding up to the nearest page size.
 * @return Number of bytes written.
 */
int print_64bit_stack(FILE * stream, const CPU & cpu, const vaddr_t & rsp,
                      const size_t count=0) throw ();

/**
 * Print a 32bit stack dump.
 * @param stream Stream to print to.
 * @param cpu CPU to do a pagetable lookup with.
 * @param rsp Stack pointer to start at.
 * @param count Number of entries to print.  Defaults to rounding up to the nearest page size.
 * @return Number of bytes written.
 */
int print_32bit_stack(FILE * stream, const CPU & cpu, const vaddr_t & rsp,
                      const size_t count=0) throw ();

/**
 * Print a code dump
 * @param stream Stream to print to.
 * @param cpu CPU to do a pagetable lookup with.
 * @param rip Instruction pointer.
 * @return Number of bytes written.
 */
int print_code(FILE * stream, const CPU & cpu, const vaddr_t & rip) throw ();


/**
 * Print a console ring.
 * @param stream Stream to print to.
 * @param cpu CPU to do a pagetable lookup with.
 * @param ring Virtual address of the console ring.
 * @param length Total length of the ring buffer.
 * @param prod Producer index, or 0 is unavailable.
 * @param cons Consumer index, or 0 if unavailable.
 * @return Number of bytes written.
 */
int print_console_ring(FILE * stream, const CPU & cpu, const vaddr_t & ring,
                       const uint64_t & length, const uint64_t & prod,
                       const uint64_t & cons) throw ();

/**
 * Dump a data region.
 * @param stream Stream to print to.
 * @param word_size Size of words (4 or 8) in bytes.
 * @param cpu CPU to do a pagetable lookup with.
 * @param start Virtual address to start dumping from.
 * @param length Total length of data to dump in bytes.
 */
int dump_data(FILE * stream, size_t word_size, const CPU & cpu, const vaddr_t & start,
              const uint64_t & length) throw ();

/**
 * Dump a 32bit data region.
 * @param stream Stream to print to.
 * @param cpu CPU to do a pagetable lookup with.
 * @param start Virtual address to start dumping from.
 * @param length Total length of data to dump in bytes.
 */
static inline int dump_32bit_data(
    FILE * stream, const CPU & cpu, const vaddr_t & start,
    const uint64_t & length) throw ()
{ return dump_data(stream, 4, cpu, start, length); }

/**
 * Dump a 64bit data region.
 * @param stream Stream to print to.
 * @param cpu CPU to do a pagetable lookup with.
 * @param start Virtual address to start dumping from.
 * @param length Total length of data to dump in bytes.
 */
static inline int dump_64bit_data(
    FILE * stream, const CPU & cpu, const vaddr_t & start,
    const uint64_t & length) throw ()
{ return dump_data(stream, 8, cpu, start, length); }

/*
 * Local variables:
 * mode: C++
 * c-set-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
