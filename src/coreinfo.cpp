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
 *  Copyright (c) 2013 Citrix Inc.
 */

/**
 * @file src/coreinfo.cpp
 * @author Andrew J. Bennieston
 */

#include "coreinfo.hpp"
#include "util/macros.hpp"

#include <cstring>

CoreInfo::CoreInfo()
 : name(NULL), data(NULL)
{}

CoreInfo::CoreInfo(const char * note_name, const size_t name_size,
                   const char * note_data, const size_t data_size)
 : name(NULL), data(NULL)
{
    name = new char[name_size + 1];
    memcpy(name, note_name, name_size);
    name[name_size] = '\0';

    data = new char[data_size + 1];
    memcpy(data, note_data, data_size);
    data[data_size] = '\0';
}

CoreInfo::CoreInfo(const size_t name_size, const size_t data_size)
    : name(NULL), data(NULL)
{
    name = new char[name_size + 1];
    memset(name, 0, name_size + 1);

    data = new char[data_size + 1];
    memset(data, 0, data_size + 1);
}

CoreInfo::~CoreInfo()
{
    this->destroy();
}

void CoreInfo::destroy()
{
    SAFE_DELETE_ARRAY(name);
    SAFE_DELETE_ARRAY(data);
}

void CoreInfo::transferOwnershipFrom(CoreInfo& other)
{
    // Clean up anything already owned by this instance
    this->destroy();

    // Assume single-threaded!
    this->name = other.name;
    other.name = NULL;
    this->data = other.data;
    other.data = NULL;
}

const char * CoreInfo::locate_key_value(const char * key) const
{
    if ( this->data == NULL )
        return NULL;

    const char * kbegin = strstr(this->data, key);
    if ( kbegin == NULL )
        return NULL;

    const char * value = strchr(kbegin, '=');
    if ( value == NULL )
        return NULL;
    else
        return ++value;
}

bool CoreInfo::lookup_key_string(
        const char * key, char * str,
        const size_t max, size_t & chars_required) const
{
    const char * idx = locate_key_value(key);
    if ( idx == NULL )
    {
        chars_required = 0;
        return false;
    }

    const char * idx2 = strchr(idx, '\n');
    if ( idx2 == NULL )
    {
        chars_required = 0;
        return false;
    }

    // Value is now between [idx,idx2)
    size_t chars_to_copy = idx2 - idx;
    if ( chars_to_copy > max - 1)
    {
        chars_required = chars_to_copy + 1;
        return false;
    }

    memset(str, 0, max);
    strncpy(str, idx, chars_to_copy);
    return true;
}

bool CoreInfo::lookup_key_vaddr(const char * key, vaddr_t& vaddr) const
{
    const char * valstr = locate_key_value(key);
    if ( valstr == NULL )
        return false;

    if ( sscanf(valstr, "%" SCNx64, &vaddr) != 1 )
        return false;
    return true;
}

bool CoreInfo::lookup_key_dec_u16(const char * key, uint16_t& value) const
{
    const char * valstr = locate_key_value(key);
    if ( valstr == NULL )
        return false;

    if ( sscanf(valstr, "%" SCNu16, &value) != 1 )
        return false;
    return true;
}

bool CoreInfo::lookup_key_dec_u32(const char * key, uint32_t& value) const
{
    const char * valstr = locate_key_value(key);
    if ( valstr == NULL )
        return false;

    if ( sscanf(valstr, "%" SCNu32, &value) != 1 )
        return false;
    return true;
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
