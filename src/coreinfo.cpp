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

/*
 * Local variables:
 * mode: C++
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
