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

#include "crashfile.hpp"
#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "main.hpp"

#include "table-decoders.hpp"

#include "Xen.h"

/**
 * @file crashfile.cpp
 * @author Andrew Cooper
 */


CrashFile::CrashFile():
    rfd(-1), efd(NULL), once(false)
{}

CrashFile::~CrashFile()
{
    if ( this -> efd )
    {
        elf_end ( this -> efd );
        this -> efd = NULL;
    }

    if ( this -> rfd >= 0 )
    {
        close( this -> rfd );
        this -> rfd = -1;
    }
}

bool CrashFile::parse(const char * path)
{
    GElf_Phdr note_hdr;
    GElf_Ehdr elf_hdr;

    // Ensure once.  We dont want to call this function multiple times
    if ( this->once )
    {
        LOG_ERROR("Called CrashFile::parse() multiple times\n");
        return false;
    }
    this->once = true;


    // Open the file
    if ( (this->rfd = open(path, O_RDONLY, NULL)) < 0)
    {
        LOG_ERROR("open() failed: %s\n", strerror(errno));
        return false;
    }

    // Ask libelf to interpret it
    if ( (this->efd = elf_begin(this->rfd, ELF_C_READ, NULL)) == NULL )
    {
        LOG_ERROR("elf_begin failed: %s\n", elf_errmsg(-1));
        return false;
    }

    // Check it is actually an ELF file
    if ( elf_kind(this->efd) != ELF_K_ELF )
    {
        LOG_ERROR("Crash file is not an ELF file\n");
        return false;
    }

    // Find the number of program headers
    if ( gelf_getehdr(this->efd, &elf_hdr) != &elf_hdr )
    {
        LOG_ERROR("gelf_getehdr failed: %s\n", elf_errmsg(-1));
        return false;
    }
    size_t phdrnum = elf_hdr.e_phnum;


    // Count the number of each type of program header
    size_t note_count=0, load_count=0;
    for ( size_t x = 0; x < phdrnum; ++x )
    {
        GElf_Phdr phdr;
        if ( gelf_getphdr(this->efd, x, &phdr) != &phdr )
        {
            LOG_ERROR("elf_getphdrnum failed: %s\n", elf_errmsg(-1));
            return false;
        }

        switch ( phdr.p_type )
        {
        case PT_LOAD:
            ++load_count;
            break;
        case PT_NOTE:
            ++note_count;
            break;
        default:
            LOG_ERROR("Unexpected program header type %"PRId32"\n", phdr.p_type);
            break;
        }
    }

    // Check that there is 1 note section (Contains CPU registers etc.)
    if ( note_count != 1 )
    {
        LOG_ERROR("Expected exactly 1 NOTE section, not %zd\n", note_count);
        return false;
    }

    // Check that there is at least 1 load section (For access to crashed RAM)
    if ( load_count < 1 )
    {
        LOG_ERROR("Expected 1 or more LOAD sections\n");
        return false;
    }

    // Inform Memory of how may regions are going to be inserted
    if ( ! memory.setup(path, load_count) )
        return false;

    for ( size_t x = 0; x < phdrnum; ++x )
    {
        GElf_Phdr phdr;
        if ( gelf_getphdr(this->efd, x, &phdr) != &phdr )
        {
            LOG_ERROR("elf_getphdrnum failed: %s\n", elf_errmsg(-1));
            return false;
        }

        switch ( phdr.p_type )
        {
        case PT_LOAD:
            memory.insert(phdr);
            break;
        case PT_NOTE:
            // Save for later.  Cant parse the notes without knowing
            // the memory regions.
            memcpy(&note_hdr, &phdr, sizeof(GElf_Phdr));
            break;
        default:
            // We already log the unexpected program header when counting.
            // fprintf(stderr, "Unexpected program header type %d\n", phdr.p_type);
            break;
        }
    }

    memory.finalise();
    this->parse_note_header(note_hdr);
    return true;
}

bool CrashFile::parse_note_header(GElf_Phdr & hdr)
{
    // notedata will be free()'d as part of the call to elf_end()
    Elf_Data * notedata;
    GElf_Nhdr nhdr;
    size_t name, desc, offset = 0, next;

    if ( !(notedata = elf_getdata_rawchunk(this->efd, hdr.p_offset,
                                           hdr.p_filesz, ELF_T_NHDR)) )
    {
        LOG_ERROR("elf_getdata_rawchunk() failed: %s", elf_errmsg(-1));
        return false;
    }

    tabdec.setup(gelf_getclass(this->efd));

    while ( (next = gelf_getnote(notedata, offset, &nhdr, &name, &desc)) )
    {
        offset = next;

        if ( (nhdr.n_type & 0xFFFFFFFFFF000000ULL ) == 0x0000000003000000ULL )
            if ( ! tabdec.decode_note(nhdr.n_type,
                                      (char*)((char*)notedata->d_buf + desc),
                                      nhdr.n_descsz) )
                LOG_ERROR("Failed to decode note\n");
    }

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
