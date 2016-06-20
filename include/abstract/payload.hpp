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
 *  Copyright (c) 2016 Citrix Inc.
 */

#ifndef __PAYLOAD_HPP__
#define __PAYLOAD_HPP__

/**
 * @file include/abstract/payload.hpp
 * @author Ross Lagerwall
 */

#include <cstddef>
#include <cstdio>

#include "abstract/pagetable.hpp"
#include "symbol-table.hpp"
#include "types.hpp"
#include "util/macros.hpp"

namespace Abstract
{

/**
 * Represents a loaded struct payload.
 */
    class Payload
    {
    public:
        /**
         * Constructor.
         * @param addr The virtual address of the payload.
         * @param xenpt Xen pagetables.
         */
        Payload(const Abstract::PageTable & xenpt, const vaddr_t & addr) :
            xenpt(xenpt), payload_addr(addr), state(0), rc(0),
            buildid(NULL), buildid_len(0),
            text_addr(0), text_end(0),
            rw_addr(0), rw_end(0),
            ro_addr(0), ro_end(0),
            symtab_ptr(0), nsyms(0), name(NULL)
        {
        };

        /// Destructor.
        virtual ~Payload()
        {
            SAFE_DELETE_ARRAY(name);
            SAFE_DELETE_ARRAY(buildid);
        };

        /**
         * Decode the payload state.
         */
        virtual void decode_state() = 0;

        /**
         * Decode the payload's symbol table and insert the symbols
         * into the given symbol table.
         * @param symtab Symbol table into which the payload's symbols
         * are inserted.
         * @return Whether the function succeeds.
         */
        virtual bool decode_symbol_table(SymbolTable & symtab);

        /**
         * Decode the given struct livepatch_symbol into a Symbol.
         * @param ptr A pointer to the struct livepatch_symbol.
         * @return A new Symbol.
         */
        virtual Symbol * decode_symbol(const vaddr_t & ptr) = 0;

        /**
         * Print information about the payload to the provided stream.
         * @param stream Stream to write to.
         * @return Number of bytes written to stream.
         */
        virtual int print_state(FILE * stream) const;

        /**
         * Print the payload's name to the provided stream.
         * @param stream Stream to write to.
         * @return Number of bytes written to stream.
         */
        virtual int print_name(FILE * stream) const;

    protected:
        virtual void decode_common();

        /// Pagetable for the context of Xen.
        const Abstract::PageTable & xenpt;

        /// Address of the struct payload represented by this object.
        vaddr_t payload_addr;

        /// state of the payload.
        uint32_t state;

        /// rc of the payload.
        int32_t rc;

        /// The payload's buildid
        uint8_t *buildid;

        /// buildid length
        uint32_t buildid_len;

        /// Start and end address of the payload's text region.
        uint64_t text_addr, text_end;

        /// Start and end address of the payload's data region.
        uint64_t rw_addr, rw_end;

        /// Start and end address of the payload's read-only data region.
        uint64_t ro_addr, ro_end;

        /// Pointer to the payload's symbol table.
        uint64_t symtab_ptr;

        /// Number of entries in the payload's symbol table.
        uint32_t nsyms;

        /// The payload's name.
        char *name;

    private:
        // @cond EXCLUDE
        Payload(const Payload &);
        Payload & operator= (const Payload &);
        // @endcond
    };

}

#endif
