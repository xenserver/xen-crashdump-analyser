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

#include "util/log.hpp"
#include "util/macros.hpp"
#include "host.hpp"
#include "memory.hpp"
#include "system.hpp"
#include "abstract/elf.hpp"

#include <getopt.h>

#include <err.h>
#include <sysexits.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <inttypes.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

/**
 * @file src/main.cpp
 * @author Andrew Cooper
 */

/// Version string
static const char * version_str = "2.2.0";

// Global variables
int verbosity = LOG_LEVEL_INFO;

// Local variables

/// Command line short options.
const static char * short_options = "hc:o:x:d:qvs";
/// Command line long options.
const static struct option long_options[] =
{
    // Help
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 0x100 },

    // Logging and verbosity
    { "quiet", no_argument, NULL, 'q' },
    { "verbose", no_argument, NULL, 'v' },

    // Files
    { "core", required_argument, NULL, 'c' },
    { "xen-symtab", required_argument, NULL, 'x' },
    { "dom0-symtab", required_argument, NULL, 'd' },

    // Directories
    { "outdir", required_argument, NULL, 'o' },

    // Additional debugging options
    { "dump-structures", no_argument, NULL, 0x101 },

    // EoL
    { NULL, 0, NULL, 0 }
};
/// Path to the xen symbol table.
static const char *xen_symtab_path;
/// Path to the dom0 symbol table.
static const char *dom0_symtab_path;
/// Default CORE crash file path.
static const char default_core_path[] = "/proc/vmcore";
/// Path to the CORE crash file.
static const char * core_path = default_core_path;
/// Log file path.
static const char * log_path = "xen-crashdump-analyser.log";
/// Path to the output directory
static const char * outdir_path = NULL;
/// Output directory descriptor
static int outdirfd = 0;
/// Working directory descriptor
static int workdirfd = 0;
/// Log file descriptor
static FILE * logfd = stderr;
/// Should we dump the Xen structures ?
static bool dump_structures = false;

/**
 * Convert a severity value to string
 * @param severity Severity value
 * @returns String representing the severity
 */
static const char * severity2str(int severity)
{
    switch ( severity )
    {
    case LOG_LEVEL_ERROR:
        return "ERROR";
    case LOG_LEVEL_WARN:
        return "WARN ";
    case LOG_LEVEL_INFO:
        return "INFO ";
    default:
    case LOG_LEVEL_DEBUG:
        return "DEBUG";
    case LOG_LEVEL_DEBUG_EXTRA:
        return "DEBUG(refs)";
    }
}

/// Additional error file descriptor for logging.
static FILE * additional_log = NULL;
void set_additional_log(FILE * fd) { additional_log = fd; }

void __log(int severity, const char * file, int line, const char * fnc, const char * fmt, ...)
{
    static char buffer[256];
    const char * sev_str = severity2str(severity);
    va_list vargs;

    va_start(vargs, fmt);
    vsnprintf(buffer, sizeof buffer - 1, fmt, vargs);
    va_end(vargs);

    if ( severity <= verbosity && logfd )
    {
        // Should we include __FILE__, __LINE__ and __fuct__ references?
        if ( verbosity >= LOG_LEVEL_DEBUG_EXTRA )
        {
            fprintf(logfd, "%s (%s:%d %s()) %s", sev_str, file, line, fnc, buffer);
            if ( additional_log && severity <= LOG_LEVEL_WARN )
                fprintf(additional_log, "%s (%s:%d %s()) %s", sev_str, file, line, fnc, buffer);
        }
        // or just the severity
        else
        {
            fprintf(logfd, "%s %s", sev_str, buffer);
            if ( additional_log && severity <= LOG_LEVEL_WARN )
                fprintf(additional_log, "%s %s", sev_str, buffer);
        }
    }

    // If this is an error message, send it stderr (if we havn't already)
    if ( severity == LOG_LEVEL_ERROR && (stderr != logfd))
        fprintf(stderr, "%s %s", sev_str, buffer);
}

/// Atexit function to close the log file descriptor
void atexit_close_log( void )
{
    if ( logfd && ( logfd != stderr ) )
    {
        fflush ( logfd );
        fclose ( logfd );
        logfd = NULL;
    }
}

FILE * fopen_in_outdir(const char * path, const char * flags)
{
    FILE * fd = NULL;
    int error;

    if ( -1 == fchdir( outdirfd ) )
    {
        LOG_ERROR("Failed to change to the output directory: %s\n",
                  strerror(errno));
        return NULL;
    }

    fd = fopen(path, flags);
    error = errno;

    if ( -1 == fchdir( workdirfd ) )
    {
        LOG_ERROR("Failed to change to working directory: %s\n",
                  strerror(errno));
    }

    errno = error;
    return fd;
}

/**
 * Print name and version.
 * @param stream Stream to write to.
 */
static void version(FILE * stream = stdout)
{
    fprintf(stream, "Xen Crashdump Analyser, version %s\n", version_str);
}

/**
 * Print usage information.
 * @param argv0 argv[0] from main() so usage can be printed with the.
 * correct context.
 * @param stream Stream to write the usage to.
 */
static void usage(char * argv0, FILE * stream = stdout)
{
    version(stream);
    fprintf(stream, "  Analyse a Xen crash from a core dump\n\n");
    fprintf(stream, "Usage: %s [options]\n", argv0);
    fprintf(stream, "Options: (* indicates required)\n\n");

// @cond - Doxygen ought to ignore these macros.
//         They are for pretty-printing the command line parameters
#define WL 15
#define L_REQ(l,d)    fprintf(stream, "    --%-*s    * %s\n", WL, l, d);
#define LS_REQ(l,s,d) fprintf(stream, "    --%-*s -%c * %s\n", WL, l, s, d);
#define L_OPT(l,d)    fprintf(stream, "    --%-*s      %s\n", WL, l, d);
#define LS_OPT(l,s,d) fprintf(stream, "    --%-*s -%c   %s\n", WL, l, s, d);

    fputs("Files:\n", stream);
    LS_OPT("core", 'c', "Core crash file.  Defaults to /proc/vmcore.");
    LS_REQ("xen-symtab", 'x', "Xen Symbol Table file.");
    LS_REQ("dom0-symtab", 'd', "Dom0 Symbol Table file.");
    putc('\n', stream);

    fputs("Directories:\n", stream);
    LS_REQ("outdir", 'o', "Directory for output files.");
    putc('\n', stream);

    fputs("General:\n", stream);
    LS_OPT("help", 'h', "This description.");
    L_OPT("version", "Display version and exit.");
    LS_OPT("quite", 'q', "Less logging.");
    LS_OPT("verbose", 'v', "More logging, accepted multiple times for extra debug logging.");
    putc('\n', stream);

    fputs("Debugging:\n", stream);
    L_OPT("dump-structures", "Hex dump key structures.");
    putc('\n', stream);

#undef L_REQ
#undef LS_REQ
#undef L_OPT
#undef LS_OPT
#undef WL
// @endcond
}

/**
 * Parse the command line arguments.
 * @param argc Command line argument count
 * @param argv Command line arguments.
 * @returns boolean indicating whether the program should continue
 */
static bool parse_commandline(int argc, char ** argv)
{
    int opt_index = 0, current = 0;

    bool have_xen_symtab = false;
    bool have_dom0_symtab = false;
    bool have_outdir = false;

    /* Show help if no command line parameters presented, rather than failing
     * with an error about unspecified parameters. */
    if ( argc == 1 )
    {
        usage(argv[0]);
        return false;
    }

    while ( current != -1 )
    {
        current = getopt_long(argc, argv, short_options,
                              long_options, &opt_index);

        switch ( current )
        {
        case -1: // No more options
            break;

        case 0x100: // --version
            version();
            return false;
            break;

        case 'c': // CORE crash file
            core_path = optarg;
            break;

        case 'o': // output directory
            outdir_path = optarg;
            have_outdir = true;
            break;

        case 'x': // xen symtab
            xen_symtab_path = optarg;
            have_xen_symtab = true;
            break;

        case 'd': // dom0 symtab
            dom0_symtab_path = optarg;
            have_dom0_symtab = true;
            break;

        case 'q': // quiet
            if ( verbosity > 0 )
                --verbosity;
            break;

        case 'v': // verbose
            if ( verbosity < LOG_LEVEL_MAX )
                ++verbosity;
            break;

        case 0x101: // Dump structures
            dump_structures = true;
            break;

        case 'h': // Help
        default: // Unrecognised
            usage(argv[0]);
        case '?': // Missing argument
        case ':': // Missing argument
            return false;
        }
    }

    if ( ! have_outdir )
    {
        printf("Required parameter {--outdir,-o} not found\n");
        return false;
    }

    if ( ! have_xen_symtab )
    {
        printf("Required parameter {--xen-symtab,-x} not found\n");
        return false;
    }

    if ( ! have_dom0_symtab )
    {
        printf("Required parameter {--dom0-symtab,-d} not found\n");
        return false;
    }

    return true;
}

/**
 * Main function.
 * @param argc Command line argument count
 * @param argv Command line arguments.
 */
int main(int argc, char ** argv)
{
    char * path_buff = NULL;
    Elf * elf = NULL;

    // Low memory environment - chances of getting std::bad_alloc are high
    try
    {
        // Log to stderr while we have no real file to log to
        logfd = stderr;

        // Parse the command line
        if ( ! parse_commandline(argc, argv) )
            return EX_USAGE;

        // Make the output dir if it doesn't exist
        if ( 0 > mkdir(outdir_path, 0700) )
        {
            if ( errno != EEXIST )
            {
                LOG_ERROR("Unable to create output directory \"%s\": %s\n",
                          outdir_path, strerror(errno));
                return EX_IOERR;
            }
        }

        // Get a handle to the current working directory
        if ( 0 > (workdirfd = open(".", O_RDONLY )))
        {
            LOG_ERROR("Unable to open working directory \"%s\": %s\n",
                      outdir_path, strerror(errno));
            return EX_IOERR;
        }

        // Get a handle to the output directory
        if ( 0 > (outdirfd = open( outdir_path, O_RDONLY )))
        {
            LOG_ERROR("Unable to open output directory \"%s\": %s\n",
                      outdir_path, strerror(errno));
            return EX_IOERR;
        }

        // Try and open the logging file
        if ( NULL == (logfd = fopen_in_outdir(log_path, "w")))
        {
            LOG_ERROR("Unable to open log file\n");
            return EX_IOERR;
        }

        // Ensure the log file gets closed if we return early
        if ( atexit(atexit_close_log) )
        {
            LOG_ERROR("call to atexit failed.  Something is very wrong\n");
            fclose(logfd);
            return EX_SOFTWARE;
        }

        // Apply line buffering to the log file
        if ( setvbuf(logfd, NULL, _IOLBF, 1024) )
        {
            LOG_ERROR("Unable to use line buffering mode for logging\n");
            return EX_IOERR;
        }

        LOG_INFO("Logging level is %s\n", severity2str(verbosity));

        // Log the command line to logfd
        if ( verbosity > 0 )
        {
            LOG_INFO("Command line:");
            for ( int x = 0; x < argc; ++x )
                fprintf(logfd, " %s", argv[x]);
            fputc('\n', logfd);
        }

        LOG_DEBUG("Opened log file '%s'\n", log_path);

        // Log the output directory
        if ( NULL == ( path_buff = realpath( outdir_path, NULL )))
        {
            LOG_ERROR("realpath failed for output directory '%s': %s\n",
                      outdir_path, strerror(errno));
            free(path_buff);
            return EX_SOFTWARE;
        }
        LOG_INFO("Output directory: %s/\n", path_buff);
        free(path_buff);

        // Log the xen symtab
        if ( NULL == ( path_buff = realpath( xen_symtab_path, NULL )))
        {
            LOG_ERROR("realpath failed for Xen symbol table path '%s': %s\n",
                      xen_symtab_path, strerror(errno));
            return EX_SOFTWARE;
        }
        LOG_INFO("Xen symbol table: %s\n", path_buff);
        free(path_buff);

        // Parse Xens symbol file
        if ( ! host.symtab.parse(xen_symtab_path, true) )
        {
            LOG_ERROR("  Failed to parse the Xen symbol table file\n");
            return EX_IOERR;
        }

        // Log the dom0 symtab
        if ( NULL == ( path_buff = realpath( dom0_symtab_path, NULL )))
        {
            LOG_ERROR("realpath failed for Dom0 symbol table path '%s': %s\n",
                      dom0_symtab_path, strerror(errno));
            return EX_SOFTWARE;
        }
        LOG_INFO("Dom0 symbol table: %s\n", path_buff);
        free(path_buff);

        gather_system_information();

        // Parse dom0s symbol file
        if ( ! host.dom0_symtab.parse(dom0_symtab_path) )
        {
            LOG_ERROR("  Failed to parse the Xen symbol table file\n");
            return EX_IOERR;
        }

        // Log the crash file
        if ( NULL == ( path_buff = realpath( core_path, NULL )))
        {
            LOG_ERROR("realpath failed for Core crash file path '%s': %s\n",
                      core_path, strerror(errno));
            free(path_buff);
            return EX_SOFTWARE;
        }
        LOG_INFO("Elf CORE crash file: %s\n", path_buff);
        free(path_buff);

        // Evaluate what kind of elf file we have
        if ( NULL == (elf = Elf::create(core_path)) )
        {
            LOG_ERROR("  Failed to parse the crash file\n");
            return EX_IOERR;
        }

        // Parse the program headers and notes
        if ( ! elf->parse() )
        {
            LOG_ERROR("  Failed to parse the crash file\n");
            SAFE_DELETE(elf);
            return EX_IOERR;
        }

        // Populate the memory regions
        if ( ! memory.setup(core_path, elf) )
        {
            LOG_ERROR("  Failed to set up memory regions from crash file\n");
            SAFE_DELETE(elf);
            return EX_SOFTWARE;
        }

        // Set up the host structures
        if ( ! host.setup(elf) )
        {
            LOG_ERROR("  Failed to set up host structures\n");
            SAFE_DELETE(elf);
            return EX_SOFTWARE;
        }

        SAFE_DELETE(elf);

        /* This ordering looks a little suspect, but it allows processing of the
         * subsequent work iff the previous work succeeds, along with fallthrough
         * error logic without gotos or returns. */
        if ( ! host.decode_xen() )
            LOG_ERROR("  Failed to decode xen structures\n");
        else if ( ! host.print_xen(dump_structures) )
            LOG_ERROR("  Failed to print xen information\n");
        else
        {
            int s = host.print_domains(dump_structures);
            LOG_DEBUG("Successfully printed %d domains\n", s);
        }
    }
    catch ( const std::bad_alloc & )
    {
        LOG_ERROR("Caught bad_alloc.  Not enough memory\n");
        return EX_SOFTWARE;
    }
    catch ( ... )
    {
        // This should never be caught, but just to be on the safe side
        LOG_ERROR("Catch wildcard triggered in %s:%d\n", __func__, __LINE__);
        abort();
    }

    LOG_INFO("COMPLETE\n");
    return EX_OK;
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
