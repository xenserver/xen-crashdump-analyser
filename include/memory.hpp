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
 *  Copyright (c) 2011,2012 Citrix Inc.
 */

#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__

/**
 * @file memory.hpp
 * @author Andrew Cooper
 */

#include <vector>

#include "types.hpp"

#include <gelf.h>

#include <cstdio>

/**
 * Memory region.
 * Directly translated from PT_LOAD program headers in the ELF CORE
 * crash file.
 */
class MemRegion
{
public:
    /// Constructor.
    MemRegion();
    /// Useful constructor from an ELF LOAD program header.
    MemRegion(const GElf_Phdr & hdr);
    /// Copy Constructor.
    MemRegion(const MemRegion & rhs);

    /// Starting physical address.
    maddr_t start;
    /// Length of region
    size_t length;
    /// Offset of memeory region into core file.
    size_t offset;

    /**
     * Operator < for sorting purposes.
     * @param rhs Right hand side of the expression.
     */
    bool operator < (const MemRegion & rhs) const;
};

/**
 * Memory
 * Provide a contigous view of memory using the ELF CORE PT_LOAD
 * regions as a reference.
 */
class Memory
{
public:
    /// Constructor.
    Memory();
    /// Destructor.
    ~Memory();

    /**
     * Hint for the number of regions.
     * Can only be used before the vector has been finalised.
     * @param path Path of the ELF CORE file.
     * @param nr_regions Number of regions to expect.
     */
    bool setup(const char * path, size_t nr_regions);

    /**
     * Insert a memory region.
     * Can only insert memory regions before the vector has been finalised.
     * @param region Region to insert.
     */
    void insert(const MemRegion & region);

    /**
     * Finalise the vector for increased read speed.
     */
    void finalise(void);

    /**
     * Read a string from machine address addr.
     * Reads n-1 bytes starting at addr, and places a NULL terminator position n in dst
     * @param addr Machine address.
     * @param dst Destination buffer.
     * @param n Length of buffer.
     * @returns number of bytes read.
     */
    size_t read_str(maddr_t addr, char * dst, size_t n) const;

    /**
     * Read a block of bytes from addr.
     * Reads n bytes starting at addr into dst.
     * @param addr Machine address.
     * @param dst Destination buffer.
     * @param n Length of buffer.
     * @returns number of bytes read.
     */
    size_t read_block(maddr_t addr, char * dst, size_t n) const;

    /**
     * Writes a text block of from addr into the specified file
     * Reads n bytes starting at addr into file.
     * @param addr Machine address.
     * @param file Destination file reference.
     * @param n Length of buffer.
     * @returns number of bytes read.
     */
    size_t write_text_block_to_file(maddr_t addr, FILE * file, size_t n) const;

protected:

    /**
     * Seek the CORE file to the byte representing the machine address addr.
     * @param addr Machine address to seek to.
     * @returns boolean indicating success or failure.
     */
    bool seek(maddr_t addr) const;

    /// Vector of memory regions.
    std::vector<MemRegion> regions;
    /// Whether the vector is finalised or not.
    bool finalised;
    /// Core File reference
    int fd;
};

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
