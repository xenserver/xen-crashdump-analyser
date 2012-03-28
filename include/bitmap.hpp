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

#ifndef __BITMAP_HPP__
#define __BITMAP_HPP__

#include <cstddef>

/**
 * @file bitmap.hpp
 * @author Andrew Cooper
 */

/**
 * Bitmap.
 * Represent a load of bits as bits in an unsigned long array.
 */
class Bitmap
{
public:
    /**
     * Constructor.
     * @param nr_bits Number of bits in the bitmap.
     * @param init Whether to set the bits or clear the bits on initialisation.
     */
    Bitmap(const size_t nr_bits, bool init=false);

    /// Destructor
    ~Bitmap();

    /**
     * Get the value of a bit in the bitmap.
     * @param bit Bit to get.
     * @returns bool Value of the bit.
     */
    bool get(const size_t bit) const;

    /**
     * Set a bit in the bitmap.
     * @param bit Bit to set.
     */
    void set(const size_t bit);

    /**
     * Clear a bit in the bitmap.
     * @param bit Bit to clear.
     */
    void clear(const size_t bit);

    /**
     * Set or clear a bit in the bitmap.
     * @param bit Bit to alter.
     * @param value Value to alter the bit to.
     */
    void update(const size_t bit, bool value);

protected:
    /// Number of bits in the bitmap
    size_t nr_bits;
    /// The bitmap itself
    unsigned long * map;
};


#endif

/*
 * Local variables:
 * mode: C++
 * c-set-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
