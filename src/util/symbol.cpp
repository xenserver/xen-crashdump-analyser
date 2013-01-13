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
 *  Copyright (c) 2011,2012 Citrix Inc.
 */

#include "util/symbol.hpp"
#include "util/macros.hpp"

#include <cstring>

/**
 * @file src/util/symbol.cpp
 * @author Andrew Cooper
 */

Symbol::Symbol(const vaddr_t a, const char t,  const char* n)
    :address(a), type(t), name(NULL)
{
    size_t nlen = strlen(n);
    this->name = new char[nlen+1];
    if ( this -> name )
    {
        strncpy(this->name, n, nlen);
        this -> name[nlen] = 0;
    }
}

Symbol::Symbol(const Symbol & rhs)
    :address(rhs.address), type(rhs.type), name(NULL)
{
    size_t nlen = strlen(rhs.name);
    this->name = new char[nlen+1];
    if ( this -> name )
    {
        strncpy(this->name, rhs.name, nlen);
        this -> name[nlen] = 0;
    }
}

Symbol::~Symbol()
{
    SAFE_DELETE_ARRAY(this->name);
}

bool Symbol::operator < (const Symbol & rhs) const
{
    return this -> address < rhs.address;
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
