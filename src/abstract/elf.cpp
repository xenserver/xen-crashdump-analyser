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
 * @file src/abstract/elf.cpp
 * @author Andrew Cooper
 */

#include "abstract/elf.hpp"
#include "arch/x86_64/elf.hpp"

#include "util/log.hpp"
#include "util/macros.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <elf.h>

Elf::Elf(int fd):
    arch(Elf::ELF_Unknown),
    nr_phdrs(0), phdrs(NULL),
    notedata(NULL), nr_notes(0), notes(NULL),
    nr_cpus(0), fd(fd)
{}

Elf::~Elf()
{
    if ( close(fd) == -1 )
        LOG_ERROR("Failed to close crash file: %s\n", strerror(errno));
    fd = -1;

    // Note entries pointers point into this->notedata,
    // so don't explicitly delete.
    SAFE_DELETE_ARRAY(this->notes);
    SAFE_DELETE_ARRAY(this->notedata);

    SAFE_DELETE_ARRAY(this->phdrs);
}

Elf * Elf::create(const char * path)
{
    int fd;
    ssize_t r;
    char ident[EI_NIDENT];

    if ( (fd = open(path, O_RDONLY, NULL)) == -1 )
    {
        LOG_ERROR("open() failed: %s\n", strerror(errno));
        return NULL;
    }

    if ( (r = read(fd, ident, sizeof ident)) == -1 )
    {
        LOG_ERROR("Failed to read elf ident: %s\n", strerror(errno));
        close(fd);
        return NULL;
    }

    if ( r != sizeof ident )
    {
        LOG_ERROR("Failed to read all of the elf ident.  Read %zu bytes instead of %zu\n",
                  r, sizeof ident);
        close(fd);
        return NULL;
    }

    if ( 0 != std::strncmp(ELFMAG, ident, SELFMAG) )
    {
        LOG_ERROR("File is not an elf file\n");
        close(fd);
        return NULL;
    }

    if ( ident[EI_DATA] != ELFDATA2LSB )
    {
        LOG_ERROR("Expected little endian elf file\n");
        close(fd);
        return NULL;
    }

    if ( ident[EI_VERSION] != EV_CURRENT )
    {
        LOG_ERROR("Expected version to be current\n");
        close(fd);
        return NULL;
    }

    if ( ident[EI_CLASS] == ELFCLASS64 )
    {
        return new x86_64Elf(fd);
    }
    // Implement 32bit elf files if really needed
    else
    {
        LOG_ERROR("Unexpected class %d\n", ident[EI_CLASS]);
        close(fd);
        return NULL;
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
