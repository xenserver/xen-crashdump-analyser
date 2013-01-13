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

#ifndef __FILE_HPP__
#define __FILE_HPP__

/**
 * @file include/util/file.hpp
 * @author Andrew Cooper
 */

#include <cstdio>

/**
 * fopen a file in the output directory.
 * Because all the parameters passed in could be relative links to the
 * required files, this program has to run from the working directory.
 * However, it needs to put files out in the output directory.
 * @param path Path of the file, relative to the output directory.
 * @param flags Open mode flags for fopen.
 * @returns fopen'd descriptor, or NULL.
 */
FILE * fopen_in_outdir(const char * path, const char * flags);


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
