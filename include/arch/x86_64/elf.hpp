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

#ifndef __X86_64_ELF_HPP__
#define __X86_64_ELF_HPP__

/**
 * @file include/arch/x86_64/elf.hpp
 * @author Andrew Cooper
 */

#include "abstract/elf.hpp"

/**
 * Parser for 64bit elf crash files
 */
class x86_64Elf : public Elf
{
public:
    /**
     * Constructor.
     * @param fd File descriptor to read from.
     */
    x86_64Elf(int fd);

    /// Destructor.
    virtual ~x86_64Elf();

    /**
     * Parse the file headers.
     * @returns boolean indicating success or failure.
     */
    virtual bool parse();

protected:

    /**
     * Parse the elf program headers.
     * @param size phentsize from ehdr, to verify against Elf64_Phdr
     * @param offset phoff from ehdr.
     * @returns boolean indicating success or failure.
     */
    bool parse_phdrs(const Elf64_Half & size, const Elf64_Off & offset);

    /**
     * Parse the elf notes.
     * @param hdr Elf Program Header containing the notes.
     * @returns boolean indicating success or failure.
     */
    bool parse_nhdrs(const ElfProgHdr & hdr);
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
