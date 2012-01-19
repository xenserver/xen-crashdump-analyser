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

/**
 * @file sym64-table-decoders.cpp
 * @author Andrew Cooper
 */

#include "sym64-table-decoders.hpp"

#include "main.hpp"

#include "types.hpp"

/// 64bit Symbol table from a 64bit CORE file
typedef struct
{
    /// Entry ID
    uint64_t id;
    /// Symbol Address
    uint64_t addr;
} x64sym64tab_t;

const uint64_t Sym64TabDecoder::invalid = 0xffffdead0000c0deULL;

x64Sym64TabDecoder::x64Sym64TabDecoder():
    table(NULL), nr_entries(0)
{
}

x64Sym64TabDecoder::~x64Sym64TabDecoder()
{
    if ( this -> table )
        delete [] this -> table;
    this -> table = NULL;
}

bool x64Sym64TabDecoder::decode(const char * buff, const size_t len)
{
    const x64sym64tab_t * symtab = (const x64sym64tab_t *)buff;
    uint64_t maxid=0;

    // If len is not a multiple of 16, something is corrupt
    if ( len & 0xf )
        return false;

    for ( size_t i = 0; i < len>>4; ++i )
        maxid = std::max(maxid, symtab[i].id);

    this->nr_entries = maxid+1;
    this->table = new uint64_t[this->nr_entries];

    for ( size_t i = 0; i < this->nr_entries; ++i )
        this->table[i] = this->invalid;

    for ( size_t i = 0; i < len>>4; ++i )
        this->table[symtab[i].id] = symtab[i].addr;


    for ( size_t i = 0; i < this->nr_entries; ++i )
        LOG_DEBUG("sym64tab[%zd] = %#016"PRIx64"\n", i, this->table[i]);

    return true;
}

size_t x64Sym64TabDecoder::length() const { return this->nr_entries; }

const uint64_t & x64Sym64TabDecoder::get(const size_t index) const
{
    if ( index < this->nr_entries )
        return this->table[index];
    return this->invalid;
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
