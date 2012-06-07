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

#ifndef __EXCEPTIONS_HPP__
#define __EXCEPTIONS_HPP__

/**
 * @file include/exceptions.hpp
 * @author Andrew Cooper
 */

#include <exception>
#include "types.hpp"
#include <stdlib.h>

/**
 * Memseek exception
 *
 * Results from failure to seek on /proc/vmcore, most likely because an attempt
 * is made to seek further than the 32bit kdump kernel can map.
 */
class memseek: public std::exception
{
public:
    /**
     * Constructor.
     * @param addr Intended address causing seek failure.
     * @param offset Offset into relevant memregion.
     */
    memseek(const maddr_t & addr, const int64_t & offset) throw();

    /// Destructor.
    virtual ~memseek() throw();

    /**
     * What is this exception.
     * @return string "memseek"
     */
    virtual const char * what() const throw();

    /**
     * Is the address outside 64GB.
     * @return boolean
     */
    virtual bool outside_64GB() const throw();

    /// Intended address
    maddr_t addr;
    /// Offset into relevant memregion.
    int64_t offset;
};

/**
 * Memread exception
 *
 * Results from failure to read a set number of bytes from /proc/vmcore
 */
class memread: public std::exception
{
public:
    /**
     * Constructor.
     * @param addr Read location causing the failure.
     * @param count Number of bytes read.
     * @param total Intended number of bytes read.
     * @param errno Error number (valid if count is -1).
     */
    memread(const maddr_t & addr, const ssize_t count, const ssize_t total, const int errno) throw();

    /// Destructor.
    virtual ~memread() throw();

    /**
     * What is this exception.
     * @return string "memread"
     */
    virtual const char * what() const throw();

    /**
     * Is the address outside 64GB.
     * @return boolean
     */
    virtual bool outside_64GB() const throw();

    /// Intended address
    maddr_t addr;
    /// Number of bytes read.
    ssize_t count;
    /// Intended number of bytes read.
    ssize_t total;
    /// Errno (valid if count is -1)
    int errno;
};

/**
 * Pagefault exception
 *
 * Results from failure to walk pagetables.
 */
class pagefault: public std::exception
{
public:
    /**
     * Constructor.
     * @param vaddr Faulting virtual address.
     * @param cr3 CR3 value used to start lookup.
     * @param level Which paging level caused the fault.
     */
    pagefault(const vaddr_t & vaddr, const uint64_t & cr3, const int level) throw();

    /// Destructor.
    virtual ~pagefault() throw();

    /**
     * What is this exception.
     * @return string "pagefault"
     */
    virtual const char * what() const throw();

    /// Faulting virtual address.
    vaddr_t vaddr;
    /// Faulting cr3.
    uint64_t cr3;
    /// Faulting level.
    int level;
};

/**
 * Validate exception
 *
 * Thrown for validation failures for xen virtual addresses.
 */
class validate: public std::exception
{
public:
    /**
     * Constructor.
     * @param vaddr Faulting virtual address.
     */
    validate(const vaddr_t & vaddr) throw();

    /// Destructor.
    virtual ~validate() throw();

    /**
     * What is this exception.
     * @return string "validate"
     */
    virtual const char * what() const throw();

    /// Invalid virtual address.
    vaddr_t vaddr;
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
