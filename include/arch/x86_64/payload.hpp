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

#ifndef __X86_64_PAYLOAD_HPP__
#define __X86_64_PAYLOAD_HPP__

/**
 * @file include/arch/x86_64/payload.hpp
 * @author Ross Lagerwall
 */

#include "abstract/payload.hpp"

namespace x86_64
{

/**
 * Represents a 64bit Xen's struct payload.
 */
    class Payload : public Abstract::Payload
    {
    public:
        /**
         * Constructor.
         * @param addr The virtual address of the payload.
         * @param xenpt Xen pagetables.
         */
        Payload(const Abstract::PageTable & xenpt, const vaddr_t & addr) :
            Abstract::Payload(xenpt, addr)
        {
        }

        /// Destructor.
        virtual ~Payload()
        {
        }

        /**
         * Decode the payload state.
         */
        virtual void decode_state();

        /**
         * Decode the given struct livepatch_symbol into a Symbol.
         * @param ptr A pointer to the struct livepatch_symbol.
         * @return A new Symbol.
         */
        virtual Symbol * decode_symbol(const vaddr_t & symtab_ptr);
    };
}

#endif
