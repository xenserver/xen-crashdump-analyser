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
 * @file table-decoders.hpp
 * @author Andrew Cooper
 */

#ifndef _X86_64_TABLE_DECODERS_HPP_
#define _X86_64_TABLE_DECODERS_HPP_

#include "table-decoder-protos.hpp"

#include "types.hpp"
#include "Xen.h"

/**
 * Table decoders.
 * Container for all table decoders
 */
class TableDecoders
{
public:
    /// Constructor
    TableDecoders();

    /// Destructor
    ~TableDecoders();

    /**
     * Set up TableDecoders for a specific architecture.
     * Can only be called once and must be called before any calls to
     * decode_note
     * @param arch CLASS of crash file
     * @returns boolean indicating success or failure
     */
    bool setup(const int arch);

    /**
     * Decode a PT_NOTE
     * @param id Note ID
     * @param buff pointer to the start of the note
     * @param len size of the note
     * @returns boolean indicating success or failure
     */
    bool decode_note(const uint64_t id,
             const char * buff,
             const size_t len);

protected:
    /// Architecture of the notes being passed.
    int arch;

    /// ensure that setup is called exactly once
    bool once;

public:
    /// String table
    StringTabDecoder * strtab;
    /// Value table
    Val64TabDecoder * val64tab;
    /// Symbol table
    Sym64TabDecoder * sym64tab;
};

#endif /* _X86_64_TABLE_DECODERS_HPP_ */

/*
 * Local variables:
 * mode: C++
 * c-set-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
