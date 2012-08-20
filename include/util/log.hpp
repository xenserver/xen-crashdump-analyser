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

#ifndef __LOG_HPP__
#define __LOG_HPP__

/**
 * @file include/util/log.hpp
 * @author Andrew Cooper
 */

#include <cstdio>

/// Logging level enumeration
enum LOG_VERBOSITY
{
    /// Error
    LOG_LEVEL_ERROR = 0,
    /// Warning
    LOG_LEVEL_WARN = 1,
    /// Information
    LOG_LEVEL_INFO = 2,
    /// Debug
    LOG_LEVEL_DEBUG = 3,
    /// Debug with extra references
    LOG_LEVEL_DEBUG_EXTRA = 4,
    /// Max level
    LOG_LEVEL_MAX = 4
};

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
 * Log function.
 * @param severity Severity of the log message.  Interacts with verbosity to work
 * out whether it should be logged or not.
 * @param file File string (__FILE__).
 * @param line File line (__LINE__).
 * @param fnc Function reference (because __FUNCTION__ is not a preprocessor macro).
 * @param fmt String format, as per printf.
 * @param ... Extra parameters for printf.
 */
void __log(int severity, const char * file, int line, const char * fnc, const char * fmt, ...);

/**
 * Set an additional destination for error logging.
 * @param fd File descriptor, or NULL to cancel.
 */
void set_additional_log(FILE * fd);

/**
 * Debug log message
 * @param fmt String format, as per printf.
 * @param args Extra arguments, as per printf.
 */
#define LOG_DEBUG(fmt, args...) \
    do { __log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__,   \
               (fmt) , ##args); } while(0)

/**
 * Info log message
 * @param fmt String format, as per printf.
 * @param args Extra arguments, as per printf.
 */
#define LOG_INFO(fmt, args...) \
    do { __log(LOG_LEVEL_INFO, __FILE__, __LINE__ , __FUNCTION__,   \
               (fmt) , ##args); } while(0)

/**
 * Warning log message
 * @param fmt String format, as per printf.
 * @param args Extra arguments, as per printf.
 */
#define LOG_WARN(fmt, args...) \
    do { __log(LOG_LEVEL_WARN, __FILE__, __LINE__ , __FUNCTION__,   \
               (fmt) , ##args); } while(0)

/**
 * Error log message
 * @param fmt String format, as per printf.
 * @param args Extra arguments, as per printf.
 */
#define LOG_ERROR(fmt, args...) \
    do { __log(LOG_LEVEL_ERROR, __FILE__, __LINE__ , __FUNCTION__,  \
               (fmt) , ##args); } while(0)

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
