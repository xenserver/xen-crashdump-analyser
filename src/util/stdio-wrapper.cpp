/*
 *  This file is part of the Xen Crashdump Analyser.
 *
 *  Xen Crashdump Analyser is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Xen Crashdump Analyser is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Xen Crashdump Analyser.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2012 Citrix Inc.
 */

#include "util/stdio-wrapper.hpp"

#include <cstdarg>
#include <cerrno>
#include "exceptions.hpp"

int FPRINTF(FILE *stream, const char *format, ...)
{
    va_list vargs;
    int ret, error;

    va_start(vargs, format);
    ret = vfprintf(stream, format, vargs);
    error = errno;
    va_end(vargs);

    if ( ret < 0 )
        throw filewrite(error);
    return ret;
}

int FPUTS(const char *s, FILE *stream)
{
    int ret = fputs(s, stream);

    if ( EOF == ret )
        throw filewrite(errno);
    return ret;
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
