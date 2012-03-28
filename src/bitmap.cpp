/*
 *  This file is part of the Xen Crashdump Analyser.
 *
 *  The Xen Crashdump Analyser is free software: you can redistribute
 *  it and/or modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation, either
 *  version 3 of the License, or (at your option) any later version.
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

#include "bitmap.hpp"

#include <cstring>


const static unsigned long BITS_PER_LONG = sizeof(unsigned long) * 8;

Bitmap::Bitmap(size_t num, bool init):
    nr_bits(num), map(NULL)
{
    size_t words = (this->nr_bits/BITS_PER_LONG)+1;
    this->map = new unsigned long[words];
    memset((void*)this->map, -!!init, words * sizeof(unsigned long));
}

Bitmap::~Bitmap()
{
    if ( this -> map )
        delete [] this -> map;
    this -> map = NULL;
    this -> nr_bits = 0;
}

bool Bitmap::get(const size_t bit) const
{
    if ( bit > this->nr_bits )
        return false;
    return ((this->map[bit/BITS_PER_LONG]) & (1<<(bit % BITS_PER_LONG)));
}

void Bitmap::set(const size_t bit)
{
    this->map[bit/BITS_PER_LONG] |= (1<<(bit % BITS_PER_LONG));
}

void Bitmap::clear(const size_t bit)
{
    this->map[bit/BITS_PER_LONG] &= ~(1<<(bit % BITS_PER_LONG));
}

void Bitmap::update(const size_t bit, bool value)
{
    if ( value )
        this->set(bit);
    else
        this->clear(bit);
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
