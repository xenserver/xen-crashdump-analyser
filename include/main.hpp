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

#ifndef __MAIN_HPP__
#define __MAIN_HPP__

/**
 * @file main.hpp
 * @author Andrew Cooper
 */
#include <cstdio>

/// We have to explicitly request the format macros...
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

//#include "symbol-table.hpp"
//#include "offset-table.hpp"

#include "crashfile.hpp"
#include "memory.hpp"
#include "table-decoders.hpp"


/// Xen Symbol table.
//extern SymbolTable xen_symtab;

/// Xen Offset table.
//extern OffsetTable xen_offsets;

/// Dom0 Symbol table.
//extern SymbolTable dom0_symtab;

/// ELF CORE file parser.
extern CrashFile crash;

/// Memory
extern Memory memory;

/// Crashnote table decoder
extern TableDecoders tabdec;

/**
 * Verbosity control for LOG macros.
 * Valid values are:
 *  - 3 DEBUG with file/line/function references
 *  - 2 DEBUG
 *  - 1 INFO (default)
 *  - 0 ERROR
 */
extern int verbosity;

/**
 * QuoteMe macro wrapper.
 * Preprocessor hackary to turn __LINE__ into a string.
 */
#define QM(x) QM2(x)

/**
 * QuoteMe macro.
 * Preprocessor hackary to turn __LINE__ into a string.
 */
#define QM2(x) #x

/// file:line reference for logging
#define REF __FILE__ ":" QM(__LINE__)

/**
 * Log function.
 * @param severity Severity of the log message.  Interacts with verbosity to work
 * out whether it should be logged or not.
 * @param ref file:line reference.
 * @param fnc Function reference (because __FUNCTION__ is not a preprocessor macro).
 * @param fmt String format, as per printf.
 * @param ... Extra parameters for printf.
 */
void __log(int severity, const char * ref, const char * fnc, const char * fmt, ...);

/**
 * Debug log message
 * @param fmt String format, as per printf.
 * @param args Extra arguments, as per printf.
 */
#define LOG_DEBUG(fmt, args...) do { __log(2, REF, __FUNCTION__, (fmt) , ##args); } while(0)

/**
 * Info log message
 * @param fmt String format, as per printf.
 * @param args Extra arguments, as per printf.
 */
#define LOG_INFO(fmt, args...) do { __log(1, REF, __FUNCTION__, (fmt) , ##args); } while(0)

/**
 * Error log message
 * @param fmt String format, as per printf.
 * @param args Extra arguments, as per printf.
 */
#define LOG_ERROR(fmt, args...) do { __log(0, REF, __FUNCTION__, (fmt) , ##args); } while(0)

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
