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

#ifndef __STDIO_WRAPPER_HPP__
#define __STDIO_WRAPPER_HPP__

/**
 * @file include/util/stdio-wrapper.hpp
 * @author Andrew Cooper
 */

#include <cstdio>

/**
 * Wrapper around fprintf, which throws filewrite exceptions if the
 * write was not successful.
 * @param stream Stream to write to.
 * @param format Format identifier.
 * @throws filewrite exception in the case of an error
 * @returns number of characters written to the stream.
 */
int FPRINTF(FILE *stream, const char *format, ...);

/**
 * Wrapper around fputs, which throws filewrite exceptions if the write
 * was not successful.
 * @param s String to write.
 * @param stream Stream to write to.
 * @throws filewrite exception in the case of an error
 * @returns a non negative number which is likely (but not guarenteed)
 * to be the number of characters written to the stream.
 */
int FPUTS(const char *s, FILE *stream);


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
