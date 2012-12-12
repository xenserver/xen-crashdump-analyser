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
 * @file src/arch/x86_64/elf.cpp
 * @author Andrew Cooper
 */

#include "arch/x86_64/elf.hpp"

#include "util/log.hpp"
#include "util/macros.hpp"
#include "types.hpp"
#include "Xen.h"

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <new>

/// Round n up to the nearest 4
#define round_up(n) (((n)+3)&~3)

x86_64Elf::x86_64Elf(int fd):Elf(fd){ this->arch = Elf::ELF_64; }
x86_64Elf::~x86_64Elf(){}

bool x86_64Elf::parse()
{
    Elf64_Ehdr ehdr;
    ssize_t r;

    if ( (-(off64_t)1) == lseek64(this->fd, 0, SEEK_SET) )
    {
        LOG_ERROR("  Failed to seek back to the beginning: %s\n", strerror(errno));
        return false;
    }

    if ( (r = read(this->fd, &ehdr, sizeof ehdr)) == -1 )
    {
        LOG_ERROR("  Failed to read elf ehdr: %s\n", strerror(errno));
        return false;
    }

    if ( r != sizeof ehdr )
    {
        LOG_ERROR("  Failed to read all of the elf ident.  Read %zu bytes instead of %zu\n",
                  r, sizeof ehdr);
        return false;
    }

    if ( sizeof ehdr != ehdr.e_ehsize )
    {
        LOG_ERROR("  Unexpected ehdr size.  Expected %zu, got %zu bytes\n",
                  sizeof ehdr, ehdr.e_ehsize);
        return false;
    }

    LOG_DEBUG("  Found %"PRIu16" section headers of size %"PRIu16" bytes at offset %"PRIx64"\n",
              ehdr.e_shnum, ehdr.e_shentsize, ehdr.e_shoff);

    LOG_DEBUG("  Found %"PRIu16" program headers of size %"PRIu16" bytes at offset %#"PRIx64"\n",
              ehdr.e_phnum, ehdr.e_phentsize, ehdr.e_phoff);

    if ( ehdr.e_phnum < 2 )
    {
        LOG_ERROR("  Expected at least 2 program headers for a crash file\n");
        return false;
    }

    this->nr_phdrs = ehdr.e_phnum;
    try
    {
        this->phdrs = new ElfProgHdr[this->nr_phdrs];
    }
    catch ( const std::bad_alloc & )
    {
        LOG_ERROR("Bad Alloc exception.  Out of memory\n");
        return false;
    }

    if ( ! this->parse_phdrs(ehdr.e_phentsize, ehdr.e_phoff) )
        return false;

    int note_count = 0, load_count = 0, unexpected_count = 0;
    const ElfProgHdr * notes = NULL;

    for ( int x = 0; x < this->nr_phdrs; ++x )
        switch ( this->phdrs[x].type )
        {
        case PT_LOAD:
            ++load_count;
            break;
        case PT_NOTE:
            ++note_count;
            notes = &this->phdrs[x];
            break;
        default:
            ++unexpected_count;
            break;
        }

    if ( note_count != 1 )
    {
        LOG_ERROR("  Expected exactly 1 note section, not %u\n", note_count);
        return false;
    }

    if ( load_count < 1 )
    {
        LOG_ERROR("  Expected at least 1 load section. Got %u\n", load_count);
        return false;
    }

    if ( unexpected_count != 0 )
    {
        LOG_ERROR("  Found %u unexpected program headers\n", unexpected_count);
        return false;
    }

    if ( ! this->parse_nhdrs(*notes) )
        return false;

    return true;
}

bool x86_64Elf::parse_phdrs(const Elf64_Half & size, const Elf64_Off & offset)
{
    Elf64_Phdr phdr;
    ssize_t r;

    if ( sizeof phdr != size )
    {
        LOG_ERROR("  Mismatch for program header size.  Expected %zu, got %"PRIu16"\n",
                  sizeof phdr, size);
        return false;
    }

    if ( (-(off64_t)1) == lseek64(this->fd, offset, SEEK_SET) )
    {
        LOG_ERROR("  Failed to seek to the program headers: %s\n", strerror(errno));
        return false;
    }

    for ( int x = 0; x < this->nr_phdrs; ++x )
    {
        if ( (r = read(this->fd, &phdr, sizeof phdr)) == -1 )
        {
            LOG_ERROR("  Failed to read elf phdr: %s\n", strerror(errno));
            return false;
        }

        if ( r != sizeof phdr )
        {
            LOG_ERROR("  Failed to read all of the program header.  Read %zu bytes instead of %zu\n",
                      r, sizeof phdr);
            return false;
        }

        this->phdrs[x].type   = phdr.p_type;
        this->phdrs[x].offset = phdr.p_offset;
        this->phdrs[x].phys   = phdr.p_paddr;
        this->phdrs[x].size   = phdr.p_filesz;
    }

    return true;
}

bool x86_64Elf::parse_nhdrs(const ElfProgHdr & hdr)
{
    Elf64_Nhdr * nhdr;
    ssize_t r, size;

    if ( hdr.type != PT_NOTE )
    {
        LOG_ERROR("  Expected note header.  Got %"PRIu64"\n", hdr.type);
        return false;
    }

    if ( hdr.size > SSIZE_MAX )
    {
        LOG_ERROR("  Note header size %"PRIu64" greater than SSIZE_MAX\n");
        return false;
    }

    size = (ssize_t)hdr.size;

    if ( (-(off64_t)1) == lseek64(this->fd, hdr.offset, SEEK_SET) )
    {
        LOG_ERROR("  Failed to seek to the note header: %s\n", strerror(errno));
        return false;
    }

    try
    {
        this->notedata = new char[size];
    }
    catch ( const std::bad_alloc & )
    {
        LOG_ERROR("Bad Alloc exception.  Out of memory\n");
        return false;
    }

    if ( (r = read(this->fd, this->notedata, size)) == -1 )
    {
        LOG_ERROR("  Failed to read elf notes: %s\n", strerror(errno));
        return false;
    }

    if ( r != size )
    {
        LOG_ERROR("  Failed to read all of the notes.  Read %zu bytes instead of %zd\n",
                  r, size);
        return false;
    }


    ssize_t index = 0;

    do
    {
        nhdr = (Elf64_Nhdr*)&this->notedata[index];
        ++this->nr_notes;
        index += sizeof *nhdr + round_up(nhdr->n_namesz) + round_up(nhdr->n_descsz);

    } while ( index < size && index >= 0 );

    if ( this->nr_notes < 3 )
    {
        LOG_ERROR("  Expected at least 3 notes.  Got %zu\n", this->nr_notes);
        return false;
    }

    try
    {
        this->notes = new ElfNote[this->nr_notes];
    }
    catch ( const std::bad_alloc & )
    {
        LOG_ERROR("Bad Alloc exception.  Out of memory\n");
        return false;
    }

    index = 0;
    int noteindex = 0;

    do
    {
        nhdr = (Elf64_Nhdr*)&this->notedata[index];

        this->notes[noteindex].name_size = nhdr->n_namesz;
        this->notes[noteindex].desc_size = nhdr->n_descsz;
        this->notes[noteindex].type = nhdr->n_type;
        this->notes[noteindex].name =
            &this->notedata[index + sizeof *nhdr];
        this->notes[noteindex].desc =
            &this->notedata[index + sizeof *nhdr + round_up(nhdr->n_namesz)];

        index += sizeof *nhdr + round_up(nhdr->n_namesz) + round_up(nhdr->n_descsz);
        ++noteindex;

    } while ( noteindex < this->nr_notes );

    int pt_count = 0, xen_core_count = 0, xen_info_count = 0;
    for ( int x = 0; x < this->nr_notes; ++x )
        switch ( this->notes[x].type )
        {
        case NT_PRSTATUS:
            ++pt_count;
            break;
        case XEN_ELFNOTE_CRASH_INFO:
            ++xen_info_count;
            break;
        case XEN_ELFNOTE_CRASH_REGS:
            ++xen_core_count;
            break;
        default:
            break;
        }

    if ( xen_info_count != 1 )
    {
        LOG_ERROR("  Expected 1 CrashXenInfo note, not %d\n", xen_info_count);
        return false;
    }

    if ( pt_count != xen_core_count )
    {
        LOG_ERROR("  Expected the same number of PR_STATUS and CrashXenCore notes.  "
                  "Got %d and %d\n", pt_count, xen_core_count);
        return false;
    }
    else
        this->nr_cpus = pt_count;

    return true;
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
