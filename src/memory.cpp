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
#include "main.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#include <cstring>
#include <algorithm>

/**
 * @file memory.cpp
 * @author Andrew Cooper
 */


MemRegion::MemRegion():
    start(0), length(0), offset(0)
{}

MemRegion::MemRegion(const GElf_Phdr & hdr):
    start(hdr.p_paddr), length(hdr.p_memsz), offset(hdr.p_offset)
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
        close( this -> fd );
        this -> fd = -1;
    }
}

bool Memory::setup(const char * path, size_t nr_regions)
{
    if ( finalised )
        return false;
    this->regions.reserve(nr_regions);

    if ( (this->fd = open(path, O_RDONLY, NULL)) < 0)
    {
        LOG_ERROR("open() failed: %s\n", strerror(errno));
        return false;
    }

    return true;
}

void Memory::insert(const MemRegion & region)
{
    if ( finalised )
        return;
    this->regions.push_back(region);
}

void Memory::finalise( void )
{
    std::sort(this->regions.begin(), this->regions.end());
    this->finalised = true;
}

size_t Memory::read_str(maddr_t addr, char * dst, size_t n) const
{
    if ( ! this->seek(addr) )
    {
        dst[0] = 0;
        return 0;
    }

    size_t num_read = read(this->fd, dst, n-1);
    dst[n] = 0;
    return num_read;
}

size_t Memory::read_block(maddr_t addr, char * dst, size_t n) const
{
    if ( ! this->seek(addr) )
        return 0;

    size_t num_read = read(this->fd, dst, n);
    return num_read;
}

size_t Memory::write_text_block_to_file(maddr_t addr, FILE * file, size_t n) const
{
    size_t num_read, num_wrote, total_written = 0;

    static const size_t BUFFER_SIZE = 4096;

    if ( ! this->seek(addr) )
        return 0;

    char * tmp = new char[BUFFER_SIZE];
    if ( ! tmp )
        return 0;

    while ( n > BUFFER_SIZE )
    {
        memset(tmp, 0, BUFFER_SIZE);

        num_read = read(this->fd, tmp, BUFFER_SIZE);
        num_wrote = fwrite(tmp, 1, strnlen(tmp, BUFFER_SIZE), file);

        n -= num_wrote; total_written += num_wrote;

        if ( num_read != BUFFER_SIZE || num_wrote != num_read )
            goto fd_error;
    }

    memset(tmp, 0, BUFFER_SIZE);

    num_read = read(this->fd, tmp, BUFFER_SIZE);
    num_wrote = fwrite(tmp, 1, strnlen(tmp, BUFFER_SIZE), file);

    n -= num_read; total_written += num_wrote;

fd_error:

    delete [] tmp;

    return total_written;
}

bool Memory::seek(maddr_t addr) const
{
    for ( std::vector<MemRegion>::const_iterator it = this->regions.begin();
          it != this->regions.end(); ++it)
    {
        if ( it->start <= addr && addr < (it->start + it->length) )
        {
            size_t foffset = addr - it->start + it->offset;
            if ( (off_t)-1 == lseek64(this->fd, foffset, SEEK_SET) )
            {
                LOG_ERROR("Failure to seek: %s\n", strerror(errno));
                return false;
            }
            return true;
        }
    }

    LOG_ERROR("Memory region for %#0x16"PRIx64" not found\n", addr);
    return false;
}

/*
 * Local variables:
 * mode: C++
 * c-set-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
