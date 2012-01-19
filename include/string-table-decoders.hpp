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
 * @file string-table-decoders.hpp
 * @author Andrew Cooper
 */


#ifndef _STRING_TABLE_DECODERS_HPP
#define _STRING_TABLE_DECODERS_HPP

#include "table-decoder-protos.hpp"

/**
 * String table decoder for 64bit CORE files
 */
class x64StringTabDecoder: public StringTabDecoder
{
public:
    /// Constructor
    x64StringTabDecoder();
    /// Destructor
    virtual ~x64StringTabDecoder();

    /**
     * Decode a raw table in memory.
     * @param buff pointer to the start of the raw table
     * @param len size of the raw table
     * @returns boolean indicating success or failure
     */
    virtual bool decode(const char * buff, const size_t len);

    /// Number of entries in the table
    virtual size_t length() const;

    /**
     * Get function
     * @param index Index into the table
     * @returns char pointer from table, or NULL if out of range or not present
     */
    virtual const char * get(const size_t index) const;

protected:
    /// Actual table
    const char ** table;
    /// Number of table entries
    size_t nr_entries;
};

#endif /* _STRING_TABLE_DECODERS_HPP */

/*
 * Local variables:
 * mode: C++
 * c-set-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
