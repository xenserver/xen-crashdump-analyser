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

#ifndef __ELF_HPP__
#define __ELF_HPP__

/**
 * @file include/abstract/elf.hpp
 * @author Andrew Cooper
 */

#include <cstring>

#include <elf.h>

/// Elf Program Header useful subset.
struct ElfProgHdr
{
    /// Program header type
    Elf64_Word type;
    /// Program header offset into file.
    Elf64_Off offset;
    /// Program header physical address
    Elf64_Addr phys;
    /// Program header size.
    Elf64_Xword size;
};


/// Elf Note.
struct ElfNote
{
    /// Size of the name string.
    uint32_t name_size,
    /// Size of the desc string.
        desc_size;
    /// Type of note.
    uint32_t type;
    /// Name string.
    const char *name,
    /// Desc string.
        *desc;
};


namespace Abstract
{

/**
 * Abstract base class for Elf file parsing.
 */
    class Elf
    {
    protected:
        /**
         * Constructor.
         * Constructor is protected to force use of Elf::create().
         * @param fd File descriptor, already opened.  We take ownership
         * of the file, and responsibility for closing it.
         */
        Elf(int fd);

    public:
        /// Destructor.
        virtual ~Elf();

        /**
         * Parse the file headers.
         * @returns boolean indicating success or failure.
         */
        virtual bool parse() = 0;

        /// Elf architecture.
        enum ElfType {
            /// Unknown.
            ELF_Unknown,
            /// 32bit.
            ELF_32,
            /// 64bit.
            ELF_64
        } arch;

        /// Number of program headers.
        int nr_phdrs;
        /// Program Headers.
        ElfProgHdr * phdrs;

        /// Contents of the note program header.
        char * notedata;
        /// Number of notes.
        int nr_notes;
        /// Notes
        ElfNote * notes;

        /// Number of cpus.
        int nr_cpus;

        /**
         * Create an Elf file parser.
         * Opens the elf file and verifies information from the ident structure
         * at the beginning.
         * @param path Path to an elf crash file.
         * @returns Architecture specific Elf class, or NULL on failure.
         */
        static Elf * create(const char * path);

    protected:
        /// File descriptor
        int fd;

    private:
        // @cond
        Elf(const Elf &);
        Elf & operator= (const Elf &);
        // @endcond
    };

}

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
