/*
 *  This file is part of the Xen Crashdump Analyser.
 *
 *  The Xen Crashdump Analyser is free software: you can redistribute
 *  it and/or modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation, either
 *  version 3 of the License, or (at your option) any later version.
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

x64Sym64TabDecoder::x64Sym64TabDecoder():
    table(NULL), valid(NULL), nr_entries(0)
{
}

x64Sym64TabDecoder::~x64Sym64TabDecoder()
{
    if ( this -> table )
        delete [] this -> table;
    this -> table = NULL;

    if ( this -> valid )
        delete this -> valid;
    this -> valid = NULL;

    this -> nr_entries = 0;
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
    if ( ! this->table )
        return false;

    // This value will be returned for ->get() of an invalid ID
    this->table[XEN_SYMTAB_INVALID] = 0ULL;

    this->valid = new Bitmap(this->nr_entries);
    if ( ! this->valid )
        return false;

    for ( size_t i = 0; i < len>>4; ++i )
    {
        uint64_t id = symtab[i].id;
        if ( XEN_SYMTAB_INVALID != id )
        {
            this->table[id] = symtab[i].addr;
            this->valid->set(id);
        }
    }

    for ( size_t i = 0; i < this->nr_entries; ++i )
        if ( this->valid->get(i) )
            LOG_DEBUG("sym64tab[%zd] = %#016"PRIx64"\n", i, this->table[i]);

    return true;
}

bool x64Sym64TabDecoder::is_valid(const size_t index) const
{
    return this->valid->get(index);
}

size_t x64Sym64TabDecoder::length() const { return this->nr_entries; }

const uint64_t & x64Sym64TabDecoder::get(const size_t index) const
{
    if ( index < this->nr_entries )
        return this->table[index];
    return this->table[XEN_SYMTAB_INVALID];
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
