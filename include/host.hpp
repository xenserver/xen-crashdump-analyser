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

#ifndef __HOST_HPP__
#define __HOST_HPP__

/**
 * @file include/host.hpp
 * @author Andrew Cooper
 */

#include <vector>

#include "symbol-table.hpp"
#include "abstract/pcpu.hpp"
#include "abstract/elf.hpp"
#include "arch/x86_64/structures.hpp"

/**
 * Host information.
 */
class Host
{
public:
    /// Constructor.
    Host();

    /// Destructor.
    ~Host();

    /**
     * Set up host information.
     *
     * @param elf Elf file
     * @return boolean indicating success or failure.
     */
    bool setup(const Elf * elf) throw();

    /**
     * Parse a Xen crash_xen_info note.
     * @param buff Buffer containing data.
     * @param len Length of the buffer in bytes.
     */
    bool parse_crash_xen_info(const char * buff, const size_t len) throw ();

    /**
     * Decode host information.
     * @return boolean indicating success or failure.
     */
    bool decode_xen() throw ();

    /**
     * Print host information to stream.
     * @return boolean indicating success or failure.
     */
    bool print_xen() throw ();

    /**
     * Decode and print domain information.
     * @return number of domains successfully printed.
     */
    int print_domains() throw ();

    /**
     * Validate a Xen virtual address.
     * @param vaddr Xen virtual address.
     * @param except Boolean indicating whether a failure should raise an
     * exception or not.  This is to cater to both boolean and exception based
     * error logic.
     * @throws validate
     * @returns boolean indicating success or failure.
     */
    bool validate_xen_vaddr(const vaddr_t & vaddr, const bool except = true);

    /// Whether setup() has been called.
    bool once;
    /// Architecture of /proc/vmcore.
    Elf::ElfType arch;
    /// Number of PCPUs.
    int nr_pcpus;
    /// PCPUs.
    PCPU ** pcpus;
    /// Idle VCPU addresses
    vaddr_t * idle_vcpus;

    /// Xen Symbol table.
    SymbolTable symtab;

    /// Dom0 Symbol table.
    SymbolTable dom0_symtab;

    /// vcpu pair.
    typedef std::pair<vaddr_t, const VCPU *> vcpu_pair;
    /// active_vcpus type.
    typedef std::vector<vcpu_pair> active_vcpus_t;
    /// Active vcpus.
    active_vcpus_t active_vcpus;

    /// Xen major version.
    int xen_major,
    /// Xen minor version.
        xen_minor;
    /// Xen extra version string.
    char *xen_extra,
    /// Xen changeset string.
        *xen_changeset,
    /// Xen compiler string.
        *xen_compiler,
    /// Xen compile date string.
        *xen_compile_date;

private:
    // @cond
    Host(const Host &);
    Host & operator= (const Host &);
    // @endcond
};

/// Host container
extern Host host;

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
