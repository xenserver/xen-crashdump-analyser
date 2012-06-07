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
 *  Copyright (c) 2012 Citrix Inc.
 */

/**
 * @file src/util/print-structures.cpp
 * @author Andrew Cooper
 */

#include "Xen.h"
#include "types.hpp"
#include "util/print-structures.hpp"
#include "util/log.hpp"
#include "util/macros.hpp"
#include "memory.hpp"

#include <limits.h>

int print_64bit_stack(FILE * o, const CPU & cpu, const vaddr_t & rsp,
                      const size_t count) throw ()
{
    int len = 0;
    const int WS = 8; // Word size in bytes
    const int WPL = 4; // Words per line
    const int mask = WS*WPL -1;

    uint64_t val;
    uint64_t sp = rsp;
    uint64_t end;
    uint64_t align;

    if ( rsp & (WS-1) )
        return len + fprintf(o, "\n\t  Stack pointer mis-aligned\n");

    if ( ! count )
        end = ((rsp | (PAGE_SIZE-1))+1);
    else
        end = rsp + count * WS;

    align = (sp & mask)/WS;
    if ( align )
    {
        len += fprintf(o, "\n\t  %016"PRIx64":", sp & ~mask);
        while ( align-- )
            len += fprintf(o, " %16s", "");
    }

    try
    {
        for ( ; sp < end; sp += WS )
        {
            if ( !(sp & mask) )
                len += fprintf(o, "\n\t  %016"PRIx64":", sp);
            memory.read64_vaddr(cpu, sp, val);
            len += fprintf(o, " %016"PRIx64, val);
        }
    }
    CATCH_COMMON

    len += fprintf(o, "\n");
    return len;
}

int print_32bit_stack(FILE * o, const CPU & cpu, const vaddr_t & rsp,
                      const size_t count) throw ()
{
    int len = 0;
    const int WS = 4; // Word size in bytes
    const int WPL = 8; // Words per line
    const int mask = WS*WPL -1;

    uint32_t val;
    uint64_t sp = rsp;
    uint64_t end;
    uint64_t align;

    if ( rsp & (WS-1) )
        return len + fprintf(o, "\t  Stack pointer mis-aligned\n");

    if ( ! count )
        end = ((rsp | (PAGE_SIZE-1))+1);
    else
        end = rsp + count * WS;

    if ( ((rsp | sp | end) & 0xffffffff00000000ULL) )
    {
        len += fprintf(o, "%016"PRIx64" %016"PRIx64" %016"PRIx64"\n", rsp, sp, end);
        return len + fprintf(o, "\t Stack pointer out of range for 32bit "
                             "Virtual Address space\n");
    }

    align = (sp & mask)/WS;
    if ( align )
    {
        len += fprintf(o, "\n\t  %08"PRIx64":", sp & ~mask);
        while ( align-- )
            len += fprintf(o, " %8s", "");

    }

    try
    {
        for ( ; sp < end; sp += WS )
        {
            if ( !(sp & mask) )
                len += fprintf(o, "\n\t  %08"PRIx64":", sp);
            memory.read32_vaddr(cpu, sp, val);
            len += fprintf(o, " %08"PRIx32, val);
        }
    }
    CATCH_COMMON

    len += fprintf(o, "\n");
    return len;
}

int print_code(FILE * o, const CPU & cpu, const vaddr_t & rip) throw ()
{
    int len = 0;
    vaddr_t ip = rip - 15;
    uint8_t d;

    len += fprintf(o, "\t  ");

    try
    {
        for ( int i = 0; i < 32; ++i )
        {
            memory.read8_vaddr(cpu, ip + i, d);
            if ( (ip + i) == rip )
                len += fprintf(o, " <%02"PRIx8">", d);
            else
                len += fprintf(o, " %02"PRIx8, d);
        }
    }
    CATCH_COMMON

    len += fprintf(o, "\n");

    return len;
}

int print_console_ring(FILE * o, const CPU & cpu,
                       const maddr_t & ring, const uint64_t & _length,
                       const uint64_t & producer, const uint64_t & consumer) throw ()
{
    int len = 0;
    int64_t prod = producer, cons = consumer, length = _length;
    ssize_t written;

    if ( _length > SSIZE_MAX )
        return len + fprintf(o, "Length(%"PRIu64") exceeds SSIZE_MAX(%zd)\n",
                             _length, (ssize_t)SSIZE_MAX);

    if ( (length & (length-1)) == 0 )
    {
        prod &= (length-1);
        cons &= (length-1);
    }

    if ( prod > length )
        return len + fprintf(o, "Producer index %"PRIu64" outside ring length %"PRIu64"\n",
                             prod, length);

    if ( cons > length )
        return len + fprintf(o, "Consumer index %"PRIu64" outside ring length %"PRIu64"\n",
                             cons, length);

    len += fprintf(o, "\n");

    try
    {
        if ( cons == 0 && prod == 0 )
        {
            written = memory.write_block_vaddr_to_file(cpu, ring, o, length);
            len += written;

            if ( written != length )
                LOG_INFO("Mismatch writing console ring to file. Written %zu bytes "
                         "of %"PRIu64"\n", written, length);
        }
        else
        {
            if ( cons > prod )
            {
                written = memory.write_block_vaddr_to_file(cpu, ring + cons,
                                                           o, length - cons);
                len += written;

                if ( (length - cons) != written )
                {
                    LOG_INFO("Mismatch writing console ring to file. Written %zu bytes "
                             "of %"PRIu64"\n", written, length - cons);
                }
                else
                {

                    written = memory.write_block_vaddr_to_file(cpu, ring, o, prod);
                    len += written;

                    if ( prod != written )
                        LOG_INFO("Mismatch writing console ring to file. Written %zu bytes "
                                 "of %"PRIu64"\n", written, prod);
                }
            }
            else
            {
                written = memory.write_block_vaddr_to_file(cpu, ring + cons, o, prod - cons);
                len += written;

                if ( (prod - cons) != written )
                    LOG_INFO("Mismatch writing console ring to file. Written %zu bytes "
                             "of %"PRIu64"\n", written, prod - cons );
            }
        }
    }
    CATCH_COMMON

    len += fprintf(o, "\n");
    return len;
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
