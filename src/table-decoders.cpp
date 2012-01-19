/*
 *  This file is part of the Xen Crashdump Analyser.
 *
 *  Foobar is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Foobar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2011,2012 Citrix Inc.
 */

/**
 * @file table-decoders.cpp
 * @author Andrew Cooper
 */

#include "table-decoders.hpp"
#include <libelf.h>

#include "string-table-decoders.hpp"
#include "val64-table-decoders.hpp"
#include "sym64-table-decoders.hpp"

#include "main.hpp"

TableDecoders::TableDecoders():
    arch(-1), once(false)
{
}

TableDecoders::~TableDecoders()
{
    if ( this->strtab )
        delete this->strtab;
    this->strtab = NULL;

    if ( this->val64tab )
        delete this->val64tab;
    this->val64tab = NULL;

    if ( this->sym64tab )
        delete this->sym64tab;
    this->sym64tab = NULL;
}

bool TableDecoders::setup(const int a)
{
    if ( this->once )
        return false;

    if ( a == ELFCLASS64 )
    {
        this->strtab = new x64StringTabDecoder();
        this->val64tab = new x64Val64TabDecoder();
        this->sym64tab = new x64Sym64TabDecoder();
    }
    else
        LOG_ERROR("Unrecognised class %d.  Can't create table decoders\n", a);

    this->once = true;
    return true;
}

bool TableDecoders::decode_note(const uint64_t id,
                                const char * buff,
                                const size_t len)
{
    if ( ! this->once )
    {
        LOG_ERROR("TableDecoders not set up\n");
        return false;
    }

    switch(id)
    {
    case XEN_ELFNOTE2_CRASH_STRINGTAB:
        return this->strtab->decode(buff, len);
        break;
    case XEN_ELFNOTE2_CRASH_VAL64TAB:
        return this->val64tab->decode(buff, len);
        break;
    case XEN_ELFNOTE2_CRASH_SYM64TAB:
        return this->sym64tab->decode(buff, len);
        break;
    default:
        return false;
    }
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
