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

#ifndef __COREINFO_HPP__
#define __COREINFO_HPP__

/**
 * @file include/coreinfo.hpp
 * @author Andrew J. Bennieston
 */

#include <cstring>

#include "types.hpp"

/**
 * Wrapper class for storing vmcoreinfo strings,
 * supporting RAII semantics.
 */
class CoreInfo {
    private:
        /**
         * Copy constructor.
         * Copy constructor is private to prevent unwanted copying.
         */
        CoreInfo(const CoreInfo&);

        /**
         * Copy assignment operator.
         * Copy assignment operator is private to prevent unwanted
         * copying.
         * @returns this
         */
        CoreInfo& operator=(const CoreInfo&);

        /// ELF note name, with trailing '\0' to behave like a C-string
        char * name;

        /// ELF note data, with trailing '\0' to behave like a C-string
        char * data;

        /**
         * Search for key and return pointer to beginning of value
         * @param key the key to search for
         * @returns pointer to beginning of value
         */
        const char * locate_key_value(const char * key) const;

    public:
        /// Constructor
        CoreInfo();

        /**
         * Constructor
         * @param note_name ELF note name
         * @param name_size length of name (excluding NULL terminator)
         * @param note_data ELF note data
         * @param data_size length of data (excluding NULL terminator)
         * @throws std::bad_alloc in the case of insufficient memory
         */
        CoreInfo(const char * note_name, const size_t name_size,
                 const char * note_data, const size_t data_size);

        /**
         * Constructor - reserve space for data to be written later
         * @param name_size length of name (excluding NULL terminator)
         * @param data_size length of data (excluding NULL terminator)
         * @throws std::bad_alloc in the case of insufficient memory
         */
        CoreInfo(const size_t name_size, const size_t data_size);

        /// Destructor
        ~CoreInfo();

        /**
         * Destroy the data owned by this CoreInfo object
         */
        void destroy();

        /**
         * Retrieve vmcoreinfo ELF note name.
         * @return C-string of name, or NULL.
         */
        char * vmcoreinfoName() { return name; }

        /**
         * Retrieve vmcoreinfo ELF note data.
         * @return C-string of newline-separated key=value pairs, or NULL.
         */
        char * vmcoreinfoData() { return data; }

        /**
         * Transfer ownership of vmcoreinfo into this object from another.
         * @param other The object to transfer ownership from
         */
        void transferOwnershipFrom(CoreInfo& other);

        /**
         * Search vmcoreinfo for a particular key, writing the associated
         * value into str as a C-string.
         *
         * If the key is not found, or is not associated with a value,
         * the function returns false and sets chars_required to 0.
         *
         * If the key was found but the destination buffer is not large
         * enough to hold the value, the function returns false and sets
         * chars_required to the buffer size that would be needed.
         *
         * Otherwise, the function writes the value into the buffer and
         * returns true. In this case, chars_required is not changed.
         *
         * @param key the key to search for.
         * @param str the destination buffer.
         * @param chars_required buffer size required if insufficient
         * @returns boolean indicating success or failure
         */
        bool lookup_key_string(
                const char * key, char * str,
                const size_t max, size_t & chars_required) const;

        /**
         * Search vmcoreinfo for a particular key, writing the associated
         * value into vaddr as a virtual address.
         * @param key the key to search for.
         * @param vaddr destination for value.
         * @returns boolean indicating success or failure.
         */
        bool lookup_key_vaddr(const char * key, vaddr_t& vaddr) const;

        /**
         * Search vmcoreinfo for a particular key, treating the associated
         * value as a decimal 16-bit integer.
         * @param key the key to search for.
         * @param value destination for value.
         * @returns boolean indicating success or failure.
         */
        bool lookup_key_dec_u16(const char * key, uint16_t& value) const;

        /**
         * Search vmcoreinfo for a particular key, treating the associated
         * value as a decimal 32-bit integer.
         * @param key the key to search for.
         * @param value destination for value.
         * @returns boolean indicating success or failure.
         */
        bool lookup_key_dec_u32(const char * key, uint32_t& value) const;
};

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
