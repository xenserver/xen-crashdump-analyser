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
 * @file src/exceptions.cpp
 * @author Andrew Cooper
 */

#include <cstring>
#include <cerrno>

#include "util/log.hpp"
#include "exceptions.hpp"

CommonError::CommonError() throw() {}
CommonError::~CommonError() throw() {}

memseek::memseek(const maddr_t & addr, const int64_t & offset) throw():
    addr(addr), offset(offset)
{}

memseek::~memseek() throw() {}

const char * memseek::what() const throw()
{
    return "memseek";
}

void memseek::log() const throw()
{
    LOG_WARN("memseek error for address 0x%016"PRIx64"\n", this->addr);
}

bool memseek::outside_64GB() const throw() { return this->addr > (1ULL << 36); }


memread::memread(const maddr_t & addr, const ssize_t count, const ssize_t total,
    const int error) throw():
    addr(addr), count(count), total(total), error(error)
{}

memread::~memread() throw() {}

const char * memread::what() const throw()
{
    return "memread";
}

void memread::log() const throw()
{
    if ( this->count == -1 )
        LOG_WARN("memread error for address 0x%016"PRIx64" - %s\n",
                 this->addr, strerror(this->error));
    else
        LOG_WARN("memread error for address 0x%016"PRIx64" - "
                 "Read %zu of intended %zu bytes\n",
                 this->addr, this->count, this->total);
}

bool memread::outside_64GB() const throw() { return this->addr > (1ULL << 36); }



pagefault::pagefault(const vaddr_t & vaddr, const uint64_t & cr3, const int level) throw():
    vaddr(vaddr), cr3(cr3), level(level)
{}

pagefault::~pagefault() throw () {}

const char * pagefault::what() const throw()
{
    return "pagefault";
}

void pagefault::log() const throw()
{
    LOG_WARN("paging error trying to follow 0x%016"PRIx64" - "
             "level %d, cr3 %016"PRIx64"\n",
             this->vaddr, this->level, this->cr3);
}


validate::validate(const vaddr_t & vaddr) throw():
    vaddr(vaddr)
{}

validate::~validate() throw () {}

const char * validate::what() const throw()
{
    return "validate";
}

void validate::log() const throw()
{
    LOG_WARN("validation error for address 0x%016"PRIx64"\n", this->vaddr);
}


filewrite::filewrite(const int error) throw():
    error(error)
{}

filewrite::~filewrite() throw() {}

const char * filewrite::what() const throw()
{
    return "filewrite";
}

void filewrite::log(const char * file) const throw()
{
    LOG_ERROR("Error writing to file '%s': %s\n", file, strerror(this->error));
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
