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

/**
 * @file src/arch/x86_64/payload.cpp
 * @author Ross Lagerwall
 */

#include "arch/x86_64/payload.hpp"

#include "abstract/xensyms.hpp"
#include "memory.hpp"
#include "util/log.hpp"
#include "util/macros.hpp"

using namespace Abstract::xensyms;

namespace x86_64
{
    void Payload::decode_state()
    {
        uint64_t text_size, rw_size, ro_size, buildid_ptr;

        decode_common();

        memory.read64_vaddr(xenpt, payload_addr + LIVEPATCH_payload_text_addr,
                            text_addr);
        memory.read64_vaddr(xenpt, payload_addr + LIVEPATCH_payload_text_size,
                            text_size);
        memory.read64_vaddr(xenpt, payload_addr + LIVEPATCH_payload_rw_addr,
                            rw_addr);
        memory.read64_vaddr(xenpt, payload_addr + LIVEPATCH_payload_rw_size,
                            rw_size);
        memory.read64_vaddr(xenpt, payload_addr + LIVEPATCH_payload_ro_addr,
                            ro_addr);
        memory.read64_vaddr(xenpt, payload_addr + LIVEPATCH_payload_ro_size,
                            ro_size);
        memory.read64_vaddr(xenpt, payload_addr + LIVEPATCH_payload_symtab,
                            symtab_ptr);
        memory.read32_vaddr(xenpt, payload_addr + LIVEPATCH_payload_nsyms,
                            nsyms);
        memory.read64_vaddr(xenpt, payload_addr + LIVEPATCH_payload_buildid,
                            buildid_ptr);
        memory.read32_vaddr(xenpt, payload_addr + LIVEPATCH_payload_buildid_len,
                            buildid_len);
        if ( buildid_len <= 128 )
        {
            buildid = new uint8_t[buildid_len];
            memory.read_block_vaddr(xenpt, buildid_ptr,
                                    (char *)buildid, buildid_len);
        }

        text_end = text_addr + text_size;
        rw_end = rw_addr + rw_size;
        ro_end = ro_addr + ro_size;
    }

    Symbol * Payload::decode_symbol(const vaddr_t & symtab_ptr)
    {
        Symbol *symbol;
        uint64_t str_ptr, value;
        size_t buflen = LIVEPATCH_payload_name_max_len +
                        LIVEPATCH_symbol_max_len;
        char *buf = new char[buflen];
        char *symname = new char[LIVEPATCH_symbol_max_len];

        memory.read64_vaddr(xenpt, symtab_ptr + LIVEPATCH_symbol_name,
                            str_ptr);
        memory.read_str_vaddr(xenpt, str_ptr,
                              symname, LIVEPATCH_symbol_max_len);
        memory.read64_vaddr(xenpt, symtab_ptr + LIVEPATCH_symbol_value,
                            value);

        // Approximate a symbol type.
        char type = (value >= text_addr && value < text_end) ? 'T' : '?';

        // Prefix the symbol name with the payload name to avoid duplicates.
        snprintf(buf, buflen, "%s.%s", name, symname);
        symbol = new Symbol(value, type, buf);

        SAFE_DELETE_ARRAY(buf);
        SAFE_DELETE_ARRAY(symname);

        return symbol;
    }
}
