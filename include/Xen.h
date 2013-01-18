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

/**
 * @file include/Xen.h
 * @author Andrew Cooper
 */

#ifndef __XEN_H__
#define __XEN_H__

#include <cstring>

/// These are structures taken from Xen header files.
/// @cond EXCLUDE
#define STACK_SIZE (4096<<3)
#define PAGE_SIZE (4096)


#define XEN_ELFNOTE_CRASH_INFO 0x1000001U
#define XEN_ELFNOTE_CRASH_REGS 0x1000002U
#define XEN_ELFNOTE_CRASH_INFO2 0x1000003U


/// @endcond

#endif /* __XEN_H__ */

/*
 * Local variables:
 * mode: C++
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
