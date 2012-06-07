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

#include "memory.hpp"
#include "util/log.hpp"

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <cstring>
#include <algorithm>

/**
 * @file src/memory.cpp
 * @author Andrew Cooper
 */

/// Buffer size for intermediate operations on larger blocks
static const ssize_t BUFFER_SIZE = 8192;

MemRegion::MemRegion():
    start(0), length(0), offset(0)
{}

MemRegion::MemRegion(const ElfProgHdr & hdr):
    start(hdr.phys), length(hdr.size), offset(hdr.offset)
{}

MemRegion::MemRegion(const MemRegion & rhs):
    start(rhs.start), length(rhs.length), offset(rhs.offset)
{}

bool MemRegion::operator < (const MemRegion & rhs) const
{
    return this->start < rhs.start;
}



Memory::Memory():
    regions(), finalised(false), fd(-1)
{}

Memory::~Memory()
{
    this -> regions . clear ( ) ;

    if ( this -> fd >= 0 )
    {
        if ( -1 == close( this -> fd ) )
            LOG_ERROR("close() failed: %s\n", strerror(errno));
        this -> fd = -1;
    }
}

bool Memory::setup(const char * path, const Elf * elf)
{
    if ( (this->fd = open(path, O_RDONLY, NULL)) == -1)
    {
        LOG_ERROR("open() failed: %s\n", strerror(errno));
        return false;
    }

    this->regions.reserve(elf->nr_phdrs-1);

    for ( int x = 0; x < elf->nr_phdrs; ++x )
        if ( elf->phdrs[x].type == PT_LOAD )
            this->regions.push_back(elf->phdrs[x]);

    std::sort(this->regions.begin(), this->regions.end());

    return true;
}

ssize_t Memory::read_str(const maddr_t & addr, char * dst, ssize_t n) const
{
    if ( ! n )
        return 0;
    dst[0] = 0;

    this->seek(addr);

    ssize_t num_read = read(this->fd, dst, n-1);
    dst[n] = 0;
    if ( num_read == -1 || num_read != n-1)
        throw memread(addr, num_read, n-1, errno);
    return strlen(dst);
}

ssize_t Memory::read_str_vaddr(const CPU & cpu, const vaddr_t & vaddr, char * dst, ssize_t n) const
{
    maddr_t maddr;
    vaddr_t end;
    cpu.pagetable_walk(vaddr, maddr, &end);
    if ( vaddr + n - 1 <= end )
        return this->read_str(maddr, dst, n);
    else
    {
        LOG_DEBUG("Correcting for passing page boundary (vaddr %016"PRIx64", maddr %016"PRIx64
                  ", end %016"PRIx64", n %zd)\n", vaddr, maddr, end, n);
        vaddr_t addr = vaddr;
        ssize_t index = 0;
        do
        {
            ssize_t nr = std::min(n, (ssize_t)(end-addr+1));
            LOG_DEBUG("Subread (vaddr %016"PRIx64", maddr %016"PRIx64", end %016"PRIx64
                      ", index %zd, nr %zd, n %zd)\n", addr, maddr, end, index, nr, n);
            this->read_block(maddr, &dst[index], nr);
            index += nr;
            addr = end+1;
            cpu.pagetable_walk(addr, maddr, &end);
            n -= nr;
        } while ( n );
        dst[n] = 0;
        return strlen(dst);
    }
}

void Memory::read8(const maddr_t & addr, uint8_t & dst) const
{
    this->seek(addr);
    ssize_t r = read(this->fd, &dst, 1);
    if ( r == -1 || 1 != r )
        throw memread(addr, r, 1, errno);
}

void Memory::read8_vaddr(const CPU & cpu, const vaddr_t & vaddr, uint8_t & dst) const
{
    maddr_t maddr;
    cpu.pagetable_walk(vaddr, maddr);
    this->read8(maddr, dst);
}


void Memory::read16(const maddr_t & addr, uint16_t & dst) const
{
    this->seek(addr);
    ssize_t r = read(this->fd, &dst, 2);
    if ( r == -1 || 2 != r )
        throw memread(addr, r, 2, errno);
}

void Memory::read16_vaddr(const CPU & cpu, const vaddr_t & vaddr, uint16_t & dst) const
{
    this->read_block_vaddr(cpu, vaddr, (char*)&dst, 2);
}

void Memory::read32(const maddr_t & addr, uint32_t & dst) const
{
    this->seek(addr);
    ssize_t r = read(this->fd, &dst, 4);
    if ( r == -1 || 4 != r )
        throw memread(addr, r, 4, errno);
}

void Memory::read32_vaddr(const CPU & cpu, const vaddr_t & vaddr, uint32_t & dst) const
{
    this->read_block_vaddr(cpu, vaddr, (char*)&dst, 4);
}

void Memory::read64(const maddr_t & addr, uint64_t & dst) const
{
    this->seek(addr);
    ssize_t r = read(this->fd, &dst, 8);
    if ( r == -1 || 8 != r )
        throw memread(addr, r, 8, errno);
}

void Memory::read64_vaddr(const CPU & cpu, const vaddr_t & vaddr, uint64_t & dst) const
{
    this->read_block_vaddr(cpu, vaddr, (char*)&dst, 8);
}

void Memory::read_block(const maddr_t & addr, char * dst, ssize_t n) const
{
    this->seek(addr);
    ssize_t r = read(this->fd, dst, n);
    if ( r == -1 || r != n )
        throw memread(addr, r, n, errno);
}

void Memory::read_block_vaddr(const CPU & cpu, const vaddr_t & vaddr, char * dst, ssize_t n) const
{
    maddr_t maddr;
    vaddr_t end;
    cpu.pagetable_walk(vaddr, maddr, &end);
    if ( vaddr + n - 1 <= end )
        this->read_block(maddr, dst, n);
    else
    {
        LOG_DEBUG("Correcting for passing page boundary (vaddr %016"PRIx64", maddr %016"PRIx64
                  ", end %016"PRIx64", n %zd)\n", vaddr, maddr, end, n);
        vaddr_t addr = vaddr;
        ssize_t index = 0;
        do
        {
            ssize_t nr = std::min(n, (ssize_t)(end-addr+1));
            LOG_DEBUG("Subread (vaddr %016"PRIx64", maddr %016"PRIx64", end %016"PRIx64
                      ", index %zd, nr %zd, n %zd)\n", addr, maddr, end, index, nr, n);
            this->read_block(maddr, &dst[index], nr);
            index += nr;
            addr = end+1;
            cpu.pagetable_walk(addr, maddr, &end);
            n -= nr;
        } while ( n );
    }
}

ssize_t Memory::write_block_to_file(const maddr_t & addr, FILE * file, ssize_t n) const
{
    ssize_t num_read, num_wrote, total_written = 0;

    if ( ! n )
        return 0;

    this->seek(addr);

    char * tmp = new char[BUFFER_SIZE];

    while ( n > BUFFER_SIZE )
    {
        num_read = read(this->fd, tmp, BUFFER_SIZE);
        if ( num_read == -1 || num_read != BUFFER_SIZE )
        {
            delete [] tmp;
            throw memread(addr, num_read, BUFFER_SIZE, errno);
        }

        num_wrote = fwrite(tmp, 1, num_read, file);
        n -= num_wrote; total_written += num_wrote;

        if ( num_read != BUFFER_SIZE || num_wrote != num_read )
        {
            delete [] tmp;
            return total_written;
        }
    }

    num_read = read(this->fd, tmp, n);
    if ( num_read == -1 || num_read != n )
    {
        delete [] tmp;
        throw memread(addr, num_read, n, errno);
    }

    num_wrote = fwrite(tmp, 1, num_read, file);
    n -= num_wrote; total_written += num_wrote;

    delete [] tmp;
    return total_written;
}

ssize_t Memory::write_block_vaddr_to_file(const CPU & cpu, const vaddr_t & vaddr, FILE * file, ssize_t n) const
{
    maddr_t maddr;
    vaddr_t end;
    cpu.pagetable_walk(vaddr, maddr, &end);
    if ( vaddr + n - 1 <= end )
        return this->write_block_to_file(maddr, file, n);
    else
    {
        LOG_DEBUG("Correcting for passing page boundary (vaddr %016"PRIx64", maddr %016"PRIx64
                  ", end %016"PRIx64", n %zd)\n", vaddr, maddr, end, n);
        vaddr_t addr = vaddr;
        ssize_t index = 0;
        do
        {
            ssize_t nr = std::min(n, (ssize_t)(end-addr+1));
            LOG_DEBUG("Subwrite (vaddr %016"PRIx64", maddr %016"PRIx64", end %016"PRIx64
                      ", index %zd, nr %zd, n %zd)\n", addr, maddr, end, index, nr, n);
            ssize_t w = this->write_block_to_file(maddr, file, nr);
            index += w;
            if ( w != nr )
                return index + w;
            addr = end+1;
            cpu.pagetable_walk(addr, maddr, &end);
            n -= nr;
        } while ( n );
        return index;
    }
}

void Memory::seek(const maddr_t & addr) const
{
    for ( std::vector<MemRegion>::const_iterator it = this->regions.begin();
          it != this->regions.end(); ++it)
    {
        if ( it->start <= addr && addr < (it->start + it->length) )
        {
            int64_t foffset = addr - it->start + it->offset;
            if ( (-(off64_t)1) == lseek64(this->fd, foffset, SEEK_SET) )
            {
                LOG_ERROR("Failure to seek: maddr 0x%016"PRIx64", foffset 0x"PRIx64": %s\n",
                          addr, foffset, strerror(errno));
                throw memseek(addr, foffset);
            }
            return;
        }
    }

    LOG_ERROR("Memory region for %#016"PRIx64" not found\n", addr);
    throw memseek(addr, 0);
}

/// Memory
Memory memory;

/*
 * Local variables:
 * mode: C++
 * c-set-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
