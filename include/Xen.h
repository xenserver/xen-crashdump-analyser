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

/**
 * @file Xen.h
 * @author Andrew Cooper
 */

#ifndef __XEN_H__
#define __XEN_H__

/// These are structures taken from Xen header files

/// @cond
#define XEN_ELFNOTE_CRASH_INFO 0x1000001
#define XEN_ELFNOTE_CRASH_INFO2 0x1000003

typedef struct {
    unsigned long xen_major_version;
    unsigned long xen_minor_version;
    unsigned long xen_extra_version;
    unsigned long xen_changeset;
    unsigned long xen_compiler;
    unsigned long xen_compile_date;
    unsigned long xen_compile_time;
    unsigned long tainted;
    unsigned long xen_phys_start;
    unsigned long dom0_pfn_to_mfn_frame_list_list;
} crash_xen_info_t;

#define XEN_ELFNOTE2_CRASH_STRINGTAB            0x3000000

#define XEN_STRINGTAB_INVALID 0
#define XEN_STRINGTAB_VERSION 1
#define XEN_STRINGTAB_CSET 2
#define XEN_STRINGTAB_COMPILE_DATE 3
#define XEN_STRINGTAB_COMPILED_BY 4
#define XEN_STRINGTAB_COMPILER 5
#define XEN_STRINGTAB_CMDLINE 6

#define XEN_ELFNOTE2_CRASH_VAL64TAB             0x3000001

#define XEN_VALTAB_INVALID 0
#define XEN_VALTAB_MAX_PAGE 1
#define XEN_VALTAB_CONRING_SIZE 2

#define XEN_ELFNOTE2_CRASH_SYM64TAB             0x3000002

#define XEN_SYMTAB_INVALID 0
#define XEN_SYMTAB_CONRING 1

/// @endcond

#endif /* __XEN_H__ */

/*
 * Local variables:
 * mode: C++
 * c-set-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
