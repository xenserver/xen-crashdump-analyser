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
#include "util/stdio-wrapper.hpp"
#include "memory.hpp"

#include <limits.h>

int print_64bit_stack(FILE * o, const PageTable & pt, const vaddr_t & rsp,
                      const size_t count)
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
        return len + FPUTS("\n\t  Stack pointer mis-aligned\n", o);

    if ( ! count )
        end = ((rsp | (PAGE_SIZE-1))+1);
    else
        end = rsp + count * WS;

    align = (sp & mask)/WS;
    if ( align )
    {
        len += FPRINTF(o, "\n\t  %016"PRIx64":", sp & ~mask);
        while ( align-- )
            len += FPRINTF(o, " %16s", "");
    }

    try
    {
        for ( ; sp < end; sp += WS )
        {
            if ( !(sp & mask) )
                len += FPRINTF(o, "\n\t  %016"PRIx64":", sp);
            memory.read64_vaddr(pt, sp, val);
            len += FPRINTF(o, " %016"PRIx64, val);
        }
    }
    catch ( const CommonError & e )
    {
        e.log();
    }

    len += FPUTS("\n", o);
    return len;
}

int print_32bit_stack(FILE * o, const PageTable & pt, const vaddr_t & rsp,
                      const size_t count)
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
        return len + FPUTS("\t  Stack pointer mis-aligned\n", o);

    if ( ! count )
        end = ((rsp | (PAGE_SIZE-1))+1);
    else
        end = rsp + count * WS;

    if ( ((rsp | sp | end) & 0xffffffff00000000ULL) )
    {
        len += FPRINTF(o, "%016"PRIx64" %016"PRIx64" %016"PRIx64"\n", rsp, sp, end);
        return len + FPUTS("\t Stack pointer out of range for 32bit "
                           "Virtual Address space\n", o);
    }

    align = (sp & mask)/WS;
    if ( align )
    {
        len += FPRINTF(o, "\n\t  %08"PRIx64":", sp & ~mask);
        while ( align-- )
            len += FPRINTF(o, " %8s", "");

    }

    try
    {
        for ( ; sp < end; sp += WS )
        {
            if ( !(sp & mask) )
                len += FPRINTF(o, "\n\t  %08"PRIx64":", sp);
            memory.read32_vaddr(pt, sp, val);
            len += FPRINTF(o, " %08"PRIx32, val);
        }
    }
    catch ( const CommonError & e )
    {
        e.log();
    }

    len += FPUTS("\n", o);
    return len;
}

int print_code(FILE * o, const PageTable & pt, const vaddr_t & rip)
{
    int len = 0;
    vaddr_t ip = rip - 15;
    uint8_t d;

    len += FPUTS("\t  ", o);

    try
    {
        for ( int i = 0; i < 32; ++i )
        {
            memory.read8_vaddr(pt, ip + i, d);
            if ( (ip + i) == rip )
                len += FPRINTF(o, " <%02"PRIx8">", d);
            else
                len += FPRINTF(o, " %02"PRIx8, d);
        }
    }
    catch ( const CommonError & e )
    {
        e.log();
    }

    len += FPUTS("\n", o);

    return len;
}

/**
 * Get log record by index.
 * idx must point to a valid message.
 *
 * @param pt Page table to use for vaddr lookup
 * @param idx Index of log record to get
 * @param log_buf vaddr of log buffer
 * @returns vaddr of log record associated with index idx.
 */
static vaddr_t log_from_idx(const PageTable & pt, uint64_t idx, vaddr_t log_buf)
{
    vaddr_t log_ptr = log_buf + idx;
    vaddr_t msglen_addr = log_ptr + 8; // &log.len
    uint16_t msglen;

    memory.read16_vaddr(pt, msglen_addr, msglen);

    /*
     * A length == 0 record is the end of buffer marker.
     * Wrap around and return the message at the start of
     * the buffer.
     */
    if ( !msglen )
        log_ptr = log_buf;

    return log_ptr;
}

/**
 * Get next record index.
 * idx must point to a valid message.
 *
 * @param pt Page table to use for vaddr lookup.
 * @param idx Index of current log record.
 * @param log_buf vaddr of log buffer.
 * @returns Index of next log record.
 */
static uint64_t log_next(const PageTable & pt, uint64_t idx, vaddr_t log_buf)
{
    vaddr_t log_ptr = log_buf + idx;
    vaddr_t msglen_addr = log_ptr + 8; // &log.len
    uint16_t msglen;

    memory.read16_vaddr(pt, msglen_addr, msglen);
    /*
     * A length == 0 record is the end of buffer marker. Wrap around and
     * read the message at the start of the buffer as *this* one, and
     * return the one after that.
     */
    if ( !msglen ) {
        msglen_addr = log_buf + 8; // &log.len
        memory.read16_vaddr(pt, msglen_addr, msglen);
        return msglen;
    }

    return idx + msglen;
}

/**
 * Convert log level into a string.
 * 
 * @param level kernel printk log level.
 * @returns C-string of human readable error level.
 */
static const char * log_level_str(uint8_t level)
{
    switch ( level )
    {
        case 0:
            return " EMERG";
        case 1:
            return " ALERT";
        case 2:
            return "  CRIT";
        case 3:
            return "   ERR";
        case 4:
            return "  WARN";
        case 5:
            return "NOTICE";
        case 6:
            return "  INFO";
        case 7:
            return " DEBUG";
        default:
            return "";
    }
}

int print_console_ring_3x(FILE * o, const PageTable & pt,
                          const vaddr_t log_buf, const uint64_t log_buf_len,
                          const uint64_t log_first_idx, const uint64_t log_next_idx)
{
    /*
     * struct log {
     *    [0] u64 ts_nsec;
     *    [8] u16 len;
     *   [10] u16 text_len;
     *   [12] u16 dict_len;
     *   [14] u8 facility;
     *   [15] u8 flags : 5;
     *   [15] u8 level : 3;
     * };
     * SIZE: 16
     */
    int len(0);
    uint64_t idx = log_first_idx;
    ssize_t written(0);
    uint16_t txtlen(0);
    int64_t text_length(0);
    uint64_t ts_nsec(0);
    uint64_t ts_sec(0), ts_frac(0);

    union {
        struct flags_t {
            uint8_t flags : 5;
            uint8_t level : 3;
        } flag_struct;

        uint8_t flag_int;
    } flags;

    try
    {
        while ( idx != log_next_idx )
        {
            vaddr_t logptr = log_from_idx(pt, idx, log_buf);
            vaddr_t txtlen_addr = logptr + 10; // &log.text_len
            vaddr_t text_addr = logptr + 16;

            memory.read64_vaddr(pt, logptr, ts_nsec);
            memory.read8_vaddr(pt, logptr + 15, flags.flag_int);
            ts_sec = ts_nsec / 1000000000;
            ts_frac = (ts_nsec % 1000000000) / 1000; /* microseconds */
            len += FPRINTF(o, "[%7"PRIu64".%.6"PRIu64"] %s: ", ts_sec, ts_frac,
                           log_level_str(flags.flag_struct.level));

            memory.read16_vaddr(pt, txtlen_addr, txtlen);
            text_length = txtlen;
            written = memory.write_block_vaddr_to_file(pt, text_addr, o, text_length);
            len += written;
            len += FPUTS("\n", o);

            if ( written != text_length )
                LOG_INFO("Mismatch writing console ring to file. Written %zu bytes "
                         "of %"PRIu64"\n", written, text_length);

            idx = log_next(pt, idx, log_buf);
            if ( idx >= log_buf_len )
            {
                len += FPRINTF(o, "\tidx of 0x%"PRIx64" bad. >= 0x%"PRIx64".\n",
                               idx, log_buf_len);
                break;
            }
        }
    }
    catch ( const CommonError & e )
    {
        e.log();
    }

    return len;
}

int print_console_ring(FILE * o, const PageTable & pt,
                       const vaddr_t & ring, const uint64_t & _length,
                       const uint64_t & producer, const uint64_t & consumer)
{
    int len = 0;
    int64_t prod = producer, cons = consumer, length = _length;
    ssize_t written;

    if ( _length > SSIZE_MAX )
        return len + FPRINTF(o, "Length(%"PRIu64") exceeds SSIZE_MAX(%zd)\n",
                             _length, (ssize_t)SSIZE_MAX);

    if ( (length & (length-1)) == 0 )
    {
        prod &= (length-1);
        cons &= (length-1);
    }

    if ( prod > length )
        return len + FPRINTF(o, "Producer index %"PRIu64" outside ring length %"PRIu64"\n",
                             prod, length);

    if ( cons > length )
        return len + FPRINTF(o, "Consumer index %"PRIu64" outside ring length %"PRIu64"\n",
                             cons, length);

    len += FPUTS("\n", o);

    try
    {
        if ( cons == 0 && prod == 0 )
        {
            LOG_DEBUG("Console ring: %"PRIu64" bytes at 0x%016"PRIx64"\n", length, ring);
            written = memory.write_block_vaddr_to_file(pt, ring, o, length);
            len += written;

            if ( written != length )
                LOG_INFO("Mismatch writing console ring to file. Written %zu bytes "
                         "of %"PRIu64"\n", written, length);
        }
        else
        {
            LOG_DEBUG("Console ring: %"PRIu64" bytes at 0x%016"PRIx64", prod %"PRId64", cons %"PRId64"\n",
                      length, ring, prod, cons);
            if ( cons >= prod )
            {
                written = memory.write_block_vaddr_to_file(pt, ring + cons,
                                                           o, length - cons);
                len += written;

                if ( (length - cons) != written )
                {
                    LOG_INFO("Mismatch writing console ring to file. Written %zu bytes "
                             "of %"PRIu64"\n", written, length - cons);
                }
                else
                {

                    written = memory.write_block_vaddr_to_file(pt, ring, o, prod);
                    len += written;

                    if ( prod != written )
                        LOG_INFO("Mismatch writing console ring to file. Written %zu bytes "
                                 "of %"PRIu64"\n", written, prod);
                }
            }
            else
            {
                written = memory.write_block_vaddr_to_file(pt, ring + cons, o, prod - cons);
                len += written;

                if ( (prod - cons) != written )
                    LOG_INFO("Mismatch writing console ring to file. Written %zu bytes "
                             "of %"PRIu64"\n", written, prod - cons );
            }
        }
    }
    catch ( const CommonError & e )
    {
        e.log();
    }

    len += FPUTS("\n", o);
    return len;
}

int dump_data(FILE * o, size_t ws, const PageTable & pt, const vaddr_t & start,
              const uint64_t & length)
{
    int len = 0;

    // Only support 32 and 64 bit dumps at the moment
    if ( ! ( ws == 4 || ws == 8 ) )
    {
        LOG_WARN("Unsupported word size '%zu' for dump_data()\n", ws);
        return 0;
    }

    // Verify that start + length does not overflow
    if ( ((-(uint64_t)1) - start) < length )
        return len + FPRINTF(o, "dump_data(): start (0x%016"PRIx64") and length "
                             "(0x%016"PRIx64") overflow the address space.\n",
                             start, length);


    for ( vaddr_t addr = start; addr < (start+length); addr += ws * 2 )
    {
        try
        {
            len += FPRINTF(o, "%04"PRIx64": ", addr - start);

            if ( ws == 4 )
            {
                union { uint32_t _32; unsigned char _8 [sizeof (uint32_t)]; } data[2];

                memory.read32_vaddr(pt, addr, data[0]._32);

                for ( size_t x = 0; x < sizeof data[0]._8; ++x )
                    len += FPRINTF(o, "%02x ", data[0]._8[x]);
                len += FPUTS(" ", o);

                memory.read32_vaddr(pt, addr+ws, data[1]._32);

                for ( size_t x = 0; x < sizeof data[1]._8; ++x )
                    len += FPRINTF(o, "%02x ", data[1]._8[x]);
                len += FPUTS(" ", o);

                len += FPRINTF(o, "0x%08"PRIx32" 0x%08"PRIx32"\n",
                               data[0]._32, data[1]._32);
            }
            else
            {
                union { uint64_t _64; unsigned char _8 [sizeof (uint64_t)]; } data[2];

                memory.read64_vaddr(pt, addr, data[0]._64);

                for ( size_t x = 0; x < sizeof data[0]._8; ++x )
                    len += FPRINTF(o, "%02x ", data[0]._8[x]);
                len += FPUTS(" ", o);

                memory.read64_vaddr(pt, addr+ws, data[1]._64);

                for ( size_t x = 0; x < sizeof data[1]._8; ++x )
                    len += FPRINTF(o, "%02x ", data[1]._8[x]);
                len += FPUTS(" ", o);

                len += FPRINTF(o, "0x%016"PRIx64" 0x%016"PRIx64"\n",
                               data[0]._64, data[1]._64);
            }
        }
        catch ( const CommonError & e )
        {
            e.log();
        }
    }

    return 0;
}

/*
 * Local variables:
 * mode: C++
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
