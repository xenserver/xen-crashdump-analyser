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

#ifndef __MACROS_HPP__
#define __MACROS_HPP__

/**
 * @file include/util/macros.hpp
 * @author Andrew Cooper
 */

/// Safe delete an object
#define SAFE_DELETE(o) do { if ((o)) delete (o); (o) = NULL; } while (0)

/// Safe delete an array
#define SAFE_DELETE_ARRAY(a) do { if ((a)) delete [] (a); (a) = NULL; } while (0)

/**
 * Common catch statements.  Presented as a macro only for code brevity in
 * otherwise large and complex functions.
 */
#define CATCH_COMMON                                                    \
    catch ( const memseek & e )                                         \
    {                                                                   \
        LOG_ERROR("memseek exception for address 0x%016"PRIx64"\n", e.addr); \
    }                                                                   \
    catch ( const memread & e )                                         \
    {                                                                   \
    if ( e.count == -1 )                                                \
        LOG_ERROR("memread exception for address 0x%016"PRIx64" - %s\n", \
                  e.addr, strerror(e.errno));                           \
    else                                                                \
        LOG_ERROR("memread exception for address 0x%016"PRIx64" - "     \
                  "Read %zu of intended %zu bytes\n", e.addr, e.count, e.total); \
    }                                                                   \
    catch ( const pagefault & e )                                       \
    {                                                                   \
        LOG_ERROR("pagefault exception for address 0x%016"PRIx64" - "   \
                  "level %d, cr3 %016"PRIx64"\n", e.vaddr, e.level, e.cr3); \
    }                                                                   \
    catch ( const validate & e )                                        \
    {                                                                   \
        LOG_ERROR("validation exception for address 0x%016"PRIx64"\n",  \
                  e.vaddr);                                             \
    }                                                                   \




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
