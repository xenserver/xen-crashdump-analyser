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

/**
 * @file string-table-decoders.cpp
 * @author Andrew Cooper
 */

#include "string-table-decoders.hpp"

#include "main.hpp"

x64StringTabDecoder::x64StringTabDecoder():
    table(NULL), nr_entries(0)
{
}

x64StringTabDecoder::~x64StringTabDecoder()
{
    if ( this -> table )
        delete [] this -> table;
    this -> table = NULL;

    this -> nr_entries = 0;
}

bool x64StringTabDecoder::decode(const char * buff, const size_t len)
{
    uint64_t id, maxid=0;
    size_t entry_len;
    const char * ptr = buff;

    if ( len & 0x7 )
    {
        LOG_ERROR("StringTab length is %zd\n", len);
        return false;
    }

    // Find the highest ID in the string table
    while ( ptr < buff + len )
    {
        // Read and record the current ID.
        maxid = std::max((uint64_t)*ptr, maxid);
        ptr += sizeof(uint64_t);

        // We have a malformed string table if any strlen is
        // longer than the entire note.
        entry_len = strnlen(ptr, len);
        if (entry_len == len)
            return false;

        // Include null terminator, then round up to a multiple of 8
        entry_len = ( entry_len + 8 ) & ~7;
        ptr += entry_len;
    }

    // IDs are 0-indexed
    this->nr_entries = maxid+1;
    this->table = new const char*[this->nr_entries];

    // This value will be returned for ->get() of an invalid ID
    this->table[XEN_STRINGTAB_INVALID] = NULL;

    ptr = buff;

    while ( ptr < buff + len )
    {
        id = (uint64_t)*ptr;
        ptr += sizeof(uint64_t);

        // ptr len already checked by previous while loop.
        entry_len = strlen(ptr);

        // Just point into the actual note data, as it will stay around
        // for the entirety of the program run.
        this->table[id] = ptr;

        entry_len = ( entry_len + 8 ) & ~7;
        ptr += entry_len;
    }

    for ( size_t i = 1; i < this->nr_entries; ++i )
        if ( this->is_valid(i) )
            LOG_DEBUG("strtab[%zd]: %s\n", i, this->table[i]);

    return true;
}

size_t x64StringTabDecoder::length() const { return this->nr_entries; }

const char * x64StringTabDecoder::get(const size_t index) const
{
    if ( !this->table || index > this->nr_entries )
        return NULL;
    return this->table[index];
}

bool x64StringTabDecoder::is_valid(const size_t index) const
{
    if ( !this->table || index > this->nr_entries )
        return false;
    return this->table[index] != NULL;
};

/*
 * Local variables:
 * mode: C++
 * c-set-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
