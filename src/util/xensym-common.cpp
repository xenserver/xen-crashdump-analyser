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
 * @file src/util/xensym-common.cpp
 * @author Andrew Cooper
 */

#include "util/log.hpp"
#include "util/xensym-common.hpp"

#include <cstring>

void insert_xensym(const xensym_t * xensyms, const char * name, vaddr_t & value)
{
    const xensym_t * sym;

    for ( sym = &xensyms[0]; sym->name; ++sym )
    {
        if ( std::strcmp(name, sym->name) != 0 )
            continue;

        if ( ! ((*sym->group) & sym->mask) )
        {
            LOG_INFO("Discarding duplicate symbol %s\n", name);
            break;
        }

        (*sym->value) = value;
        (*sym->group) &= ~sym->mask;

        break;
    }
}

bool _required_xensyms(const xensym_t * xensyms, const uint64_t * group)
{
    const xensym_t * sym;
    bool ret = true;

    /* In the common case that the group bitmask is 0, we are certain that
     * group xensyms are present, so skip scanning the xensyms list. */
    if ( ! *group )
        return ret;

    for ( sym = &xensyms[0]; sym->name; ++sym )
        if ( ( group == sym->group ) && ( *group & sym->mask ) )
        {
            ret = false;
            LOG_ERROR("Missing required xensym %s\n", sym->name);
        }

    return ret;
}
