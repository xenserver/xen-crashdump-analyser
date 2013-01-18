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

#ifndef __XENSYM_COMMON_HPP__
#define __XENSYM_COMMON_HPP__


/**
 * @file include/util/xensym-common.hpp
 * @author Andrew Cooper
 */

#include "types.hpp"
#include <cstring>

/**
 * Macro for declaring a group of related symbols.
 *
 * For use in header files only.
 * @param g Xensym group name.
 */
#define DECLARE_XENSYM_GROUP(g) \
    extern uint64_t required_##g##_symbols

/**
 * Macro for defining a group of related symbols.
 *
 * For use in source files only.
 *
 * It sets up the public symbol 'required_\<group\>_symbols', and a private
 * variable used by XENSYM() to set up the relevant masks.
 *
 * The XENSYM()'s (ab)use of mask and the comma operator will work for up to
 * 63 symbols per group, which is sufficent for now.
 *
 * @param g Xensym group name.
 */
#define DEFINE_XENSYM_GROUP(g) \
    uint64_t required_##g##_symbols; static uint64_t _meta_##g##_sym_mask = 1

/**
 * Macro to help setting up the required symbols table.
 *
 * It (ab)uses the comma operator and the two variables from the GROUP() macro
 * to count the total number of symbols, and create bitmasks for tracking
 * which have been found in the symbol table.
 *
 * @param g Xensym group.
 * @param n Xensym name.
 */
#define XENSYM(g, n) { #n , &(n) , &(required_##g##_symbols) ,          \
            (_meta_##g##_sym_mask <<= 1, required_##g##_symbols = _meta_##g##_sym_mask - 1, \
             _meta_##g##_sym_mask >> 1) }

/**
 * XenSym container.
 *
 * For specific symbols and offset information from the Xen symbol table.
 */
typedef struct xensym
{
    /// Name.
    const char * name;
    /// Value.
    vaddr_t * value;
    /// Symbol group.
    uint64_t * group;
    /// Found mask.
    uint64_t mask;
} xensym_t;

/// Null terminator for a xensym list.
#define XENSYM_NULL { NULL, NULL, NULL, 0ull }


/**
 * Insert a symbol or offset from the Xen symbol table into a specific xensym list.
 *
 * @param xensyms Null terminated list of xensym containers.
 * @param name Symbol or offset name.
 * @param value Value or address of symbol or offset.
 */
void insert_xensym(const xensym_t * xensyms, const char * name, vaddr_t & value);

#endif

/*
 * Local variables:
 * mode: C++
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
