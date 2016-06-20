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
 * @file src/abstract/payload.cpp
 * @author Ross Lagerwall
 */

#include "abstract/payload.hpp"

#include "abstract/xensyms.hpp"
#include "memory.hpp"
#include "util/log.hpp"
#include "util/macros.hpp"
#include "util/stdio-wrapper.hpp"

using namespace Abstract::xensyms;

namespace Abstract
{
    void Payload::decode_common()
    {
        memory.read32_vaddr(xenpt, payload_addr + LIVEPATCH_payload_state,
                            state);
        memory.read32_vaddr(xenpt, payload_addr + LIVEPATCH_payload_rc,
                            (uint32_t &)rc);
        name = new char[LIVEPATCH_payload_name_max_len];
        memory.read_str_vaddr(xenpt, payload_addr + LIVEPATCH_payload_name,
                              name, LIVEPATCH_payload_name_max_len);
    }

    bool Payload::decode_symbol_table(SymbolTable & symtab)
    {
        if ( nsyms > 1024 )
        {
            LOG_ERROR("Too many payloads\n");
            return false;
        }

        /*
         * Add symbols representing the start and end of the payload's
         * text region.
         */
        size_t buflen = strlen(name) + 7 + 1;
        char *buf = new char[buflen];
        snprintf(buf, buflen, "%s._stext", name);
        symtab.insert(new Symbol(text_addr, 'T', buf));
        snprintf(buf, buflen, "%s._etext", name);
        symtab.insert(new Symbol(text_end, 'T', buf));
        SAFE_DELETE_ARRAY(buf);

        symtab.add_text_region(text_addr, text_end);

        for ( unsigned int i = 0; i < nsyms; i++ )
        {
            symtab.insert(decode_symbol(symtab_ptr));
            symtab_ptr += LIVEPATCH_symbol_sizeof;
        }

        return true;
    }

    int Payload::print_state(FILE * stream) const
    {
        int len = 0;

        len += FPRINTF(stream, "  Payload %s:\n", name);
        len += FPRINTF(stream, "    at address 0x%016"PRIx64"\n", payload_addr);
        len += FPRINTF(stream, "    state %d\n", state);
        len += FPRINTF(stream, "    rc %d\n", rc);
        if ( buildid )
        {
            len += FPRINTF(stream, "    buildid ");

            for ( unsigned int i = 0; i < buildid_len; i++ )
                len += FPRINTF(stream, "%02x", buildid[i]);

            len += FPUTS("\n", stream);
        }
        len += FPRINTF(stream, "    text [0x%016"PRIx64"-0x%016"PRIx64"]\n",
                       text_addr, text_end - 1);
        if ( rw_end > rw_addr )
            len += FPRINTF(stream, "    rw   [0x%016"PRIx64"-0x%016"PRIx64"]\n",
                           rw_addr, rw_end - 1);
        if ( ro_end > ro_addr )
            len += FPRINTF(stream, "    ro   [0x%016"PRIx64"-0x%016"PRIx64"]\n",
                           ro_addr, ro_end - 1);

        return len;
    }

    int Payload::print_name(FILE * stream) const
    {
        return FPRINTF(stream, "  %s\n", name);
    }
}
