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

#ifndef __SYMBOL_HPP__
#define __SYMBOL_HPP__

#include "types.hpp"

/**
 * @file include/util/symbol.hpp
 * @author Andrew Cooper
 */

/**
 * Symbol structure.
 * Used to store a single symbol read from the static symbol map files
 * in /boot, or pulled from the kernel module symbol table in memory.
 */
class Symbol
{
public:

    /**
     * Regular Constructor.
     * @param address Virtual address.
     * @param type What sort of symbol this is.
     * @param name Symbol name.  For multiple symbols pointing to the
     * same address, this may become a comma separated string of
     * different symbol names
     */
    Symbol(const vaddr_t address, const char type,  const char * name);

    /**
     * Copy Constructor.
     * Implements deep copy.
     * @param rhs Object to be copied from.
     */
    Symbol(const Symbol & rhs);

    /**
     * Destructor.
     */
    ~Symbol();

    /**
     * Overloaded less-than operator.
     * For sorting within stl containers.
     * @param rhs Object to be compared to.
     * @return boolean.
     */
    bool operator < (const Symbol & rhs) const;

    /// Virtual address.
    vaddr_t address;

    /// Type of symbol.
    char type;

    /**
     * Name(s) of symbol.
     * May be a comma separated list of symbol names if they alias the
     * same virtual address in a symbol table.
     */
    char * name;

private:
    // @cond
    Symbol & operator=(const Symbol &);
    // @endcond
};

#endif

/*
 * Local variables:
 * mode: C++
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
