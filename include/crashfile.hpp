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

#ifndef __CRASHFILE_HPP__
#define __CRASHFILE_HPP__

/**
 * @file crashfile.hpp
 * @author Andrew Cooper
 */

#include <libelf.h>
#include <gelf.h>

/**
 * Crash File.
 * Used to provide RAII for libelf constructs, and to parse headers
 * for /proc/vmcore.
 */
class CrashFile
{
public:
    /// Constructor.
    CrashFile();
    /// Destructor.
    ~CrashFile();

    /**
     * Parse an Elf CORE file.
     * @param path Path to the file.
     * @return boolean indicating success or failure.
     */
    bool parse(const char * path);

    /**
     * Parse the program notes of an Elf CORE file.
     * @param hdr Gelf program header section
     * @return boolean indicating success or failure.
     */
    bool parse_note_header(GElf_Phdr & hdr);

protected:
    /// Real file descriptor, from open().
    int rfd;
    /// Libelf descriptor, based on rfd.
    Elf * efd;
    /// Ensure parse is only called once.
    bool once;
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
