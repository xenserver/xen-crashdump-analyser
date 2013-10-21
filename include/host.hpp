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

#ifndef __HOST_HPP__
#define __HOST_HPP__

/**
 * @file include/host.hpp
 * @author Andrew Cooper
 */

#include <vector>

#include "coreinfo.hpp"
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
    bool setup(const Abstract::Elf * elf);

    /**
     * Parse a Xen crash_xen_info note.
     * @param buff Buffer containing data.
     * @param len Length of the buffer in bytes.
     * @return boolean indicating success or failure.
     */
    bool parse_crash_xen_info(const char * buff, const size_t len);

    /**
     * Decode host information.
     * @return boolean indicating success or failure.
     */
    bool decode_xen();

    /**
     * Print host information to stream.
     * @param dump_structures boolean indicating whether the Xen structures should be dumped.
     * @return boolean indicating success or failure.
     */
    bool print_xen(bool dump_structures);

    /**
     * Decode and print domain information.
     * @param dump_structures boolean indicating whether the Xen structures should be dumped.
     * @return number of domains successfully printed.
     */
    int print_domains(bool dump_structures);

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

    /**
     * Get a usable set of Xen pagetables.
     * @throws Validate if no pcpus have suitable pagetables.
     * @returns Pagetables.
     */
    const Abstract::PageTable & get_xenpt() const;

    /**
     * Parse a VMCOREINFO ELF note.
     * @param note The ELF note to parse.
     * @returns boolean indicating success or failure
     */
    bool parse_vmcoreinfo(const ElfNote& note);

    /// Whether setup() has been called.
    bool once;
    /// Architecture of /proc/vmcore.
    Abstract::Elf::ElfType arch;
    /// Number of PCPUs.
    int nr_pcpus;
    /// PCPUs.
    Abstract::PCPU ** pcpus;
    /// Idle VCPU addresses
    vaddr_t * idle_vcpus;

    /// Xen Symbol table.
    SymbolTable symtab;

    /// Dom0 Symbol table.
    SymbolTable dom0_symtab;

    /// vcpu pair.
    typedef std::pair<vaddr_t, const Abstract::VCPU *> vcpu_pair;
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
    /// Is Xen a debug build?
    bool debug_build;
    /// Have we got the virtual address information from the symbol table?
    bool can_validate_xen_vaddr;

    /// Xen vmcoreinfo
    CoreInfo xen_vmcoreinfo;

    /// dom0 vmcoreinfo
    CoreInfo dom0_vmcoreinfo;

private:
    // @cond EXCLUDE
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
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
