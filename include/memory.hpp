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

#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__

/**
 * @file include/memory.hpp
 * @author Andrew Cooper
 */

#include <vector>

#include "types.hpp"
#include "exceptions.hpp"
#include "abstract/cpu.hpp"
#include "abstract/elf.hpp"

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
    MemRegion(const ElfProgHdr & hdr);
    /// Copy Constructor.
    MemRegion(const MemRegion & rhs);

    /// Starting physical address.
    maddr_t start;
    /// Length of region
    uint64_t length;
    /// Offset of memory region into core file.
    uint64_t offset;

    /**
     * Operator < for sorting purposes.
     * @param rhs Right hand side of the expression.
     */
    bool operator < (const MemRegion & rhs) const;
};

/**
 * Memory
 * Provide a contiguous view of memory using the ELF CORE PT_LOAD
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
     * Set up the memory regions
     * @param path Path of the ELF CORE file.
     * @param elf Elf parser.
     */
    bool setup(const char * path, const Elf * elf);

    /**
     * Read a string from machine address addr.
     * Reads n-1 bytes starting at addr, and places a NULL terminator position n in dst
     * @param addr Machine address.
     * @param dst Destination buffer.
     * @param n Length of buffer.
     * @returns strlen(dst)
     */
    ssize_t read_str(const maddr_t & addr, char * dst, ssize_t n) const;

    /**
     * Read a string from virtual address addr.
     * Reads n-1 bytes starting at addr, and places a NULL terminator position n in dst
     * @param cpu CPU to perform a pagetable walk with.
     * @param addr Virtual address.
     * @param dst Destination buffer.
     * @param n Length of buffer.
     * @returns strlen(dst)
     */
    ssize_t read_str_vaddr(const CPU & cpu, const vaddr_t & addr, char * dst, ssize_t n) const;

    /**
     * Read a block of bytes from addr.
     * Reads n bytes starting at addr into dst.
     * @param addr Machine address.
     * @param dst Destination buffer.
     * @param n Length of buffer.
     */
    void read_block(const maddr_t & addr, char * dst, ssize_t n) const;

    /**
     * Read a block of bytes from addr.
     * Reads n bytes starting at addr into dst.
     * @param cpu CPU to perform a pagetable walk with.
     * @param addr Virtual address.
     * @param dst Destination buffer.
     * @param n Length of buffer.
     */
    void read_block_vaddr(const CPU & cpu, const vaddr_t & addr, char * dst, ssize_t n) const;


    /**
     * Read a 8 bit integer from addr.
     * Reads 1 bytes from addr into dst.
     * @param addr Machine address.
     * @param dst Destination integer.
     */
    void read8(const maddr_t & addr, uint8_t & dst) const;

    /**
     * Read a 8 bit integer from addr.
     * Reads 1 bytes from addr into dst.
     * @param cpu CPU to perform a pagetable walk with.
     * @param addr Virtual address.
     * @param dst Destination integer.
     */
    void read8_vaddr(const CPU & cpu, const vaddr_t & addr, uint8_t & dst) const;

    /**
     * Read a 16 bit integer from addr.
     * Reads 2 bytes from addr into dst.
     * @param addr Machine address.
     * @param dst Destination integer.
     */
    void read16(const maddr_t & addr, uint16_t & dst) const;

    /**
     * Read a 16 bit integer from addr.
     * Reads 2 bytes from addr into dst.
     * @param cpu CPU to perform a pagetable walk with.
     * @param addr Virtual address.
     * @param dst Destination integer.
     */
    void read16_vaddr(const CPU & cpu, const vaddr_t & addr, uint16_t & dst) const;

    /**
     * Read a 32 bit integer from addr.
     * Reads 4 bytes from addr into dst.
     * @param addr Machine address.
     * @param dst Destination integer.
     */
    void read32(const maddr_t & addr, uint32_t & dst) const;

    /**
     * Read a 32 bit integer from addr.
     * Reads 4 bytes from addr into dst.
     * @param cpu CPU to perform a pagetable walk with.
     * @param addr Virtual address.
     * @param dst Destination integer.
     */
    void read32_vaddr(const CPU & cpu, const vaddr_t & addr, uint32_t & dst) const;

    /**
     * Read a 32 bit integer from addr.
     * Reads 8 bytes from addr into dst.
     * @param addr Machine address.
     * @param dst Destination integer.
     */
    void read64(const maddr_t & addr, uint64_t & dst) const;

    /**
     * Read a 32 bit integer from addr.
     * Reads 8 bytes from addr into dst.
     * @param cpu CPU to perform a pagetable walk with.
     * @param addr Virtual address.
     * @param dst Destination integer.
     */
    void read64_vaddr(const CPU & cpu, const vaddr_t & addr, uint64_t & dst) const;

    /**
     * Writes a block of from addr into the specified file.
     * Reads n bytes starting at addr into file.
     * @param addr Machine address.
     * @param file Destination file reference.
     * @param n Length of buffer.
     * @returns number of bytes read.
     */
    ssize_t write_block_to_file(const maddr_t & addr, FILE * file, ssize_t n) const;

    /**
     * Writes a block of from addr into the specified file.
     * Reads n bytes starting at addr into file.
     * @param cpu CPU to perform a pagetable walk with.
     * @param addr Virtual address.
     * @param file Destination file reference.
     * @param n Length of buffer.
     * @returns number of bytes read.
     */
    ssize_t write_block_vaddr_to_file(const CPU & cpu, const vaddr_t & addr, FILE * file, ssize_t n) const;

protected:

    /**
     * Seek the CORE file to the byte representing the machine address addr.
     * @param addr Machine address to seek to.
     * @returns boolean indicating success or failure.
     */
    void seek(const maddr_t & addr) const;

    /// Vector of memory regions.
    std::vector<MemRegion> regions;
    /// Whether the vector is finalised or not.
    bool finalised;
    /// Core File reference
    int fd;
};

/// Memory
extern Memory memory;

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
