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

#ifndef __X86_64_SVCPU_HPP__
#define __X86_64_SVCPU_HPP__

/**
 * @file include/arch/x86_64/vcpu.hpp
 * @author Andrew Cooper
 */

#include "abstract/vcpu.hpp"

#include "arch/x86_64/structures.hpp"

/**
 * Class for parsing a 64bit Xen's virtual CPUs.
 *
 * The parsing is a little complicated because of how active VCPUs at the time
 * of crash have their state stored.
 */
class x86_64VCPU : public VCPU
{
public:
    /// Constructor.
    x86_64VCPU();

    /// Destructor.
    virtual ~x86_64VCPU();


    /**
     * Translate a virtual address to a physical address using this cpus cr3 value.
     * @param vaddr Virtual address to translate
     * @param maddr Machine address result of translation
     * @param page_end If non-null, variable to be filled with the last virtual address
     */
    virtual void pagetable_walk(const vaddr_t & vaddr, maddr_t & maddr,
                                vaddr_t * page_end) const;

    /**
     * Parse basic information from a Xen struct vcpu.
     * Pulls non-register information from Xen's struct vcpu, and associated
     * struct domain.
     * @param addr Xen pointer to a struct vcpu.
     * @param cpu CPU with which pagetable lookups can be performed.
     * @return boolean indicating success or failure.
     */
    virtual bool parse_basic(const vaddr_t & addr, const CPU & cpu) throw ();

    /**
     * Parse register information from a Xen per-cpu structure.
     *
     * This means that this particular VCPU had active register state on the Xen
     * per-cpu stack, meaning that the register values stored in the struct vcpu
     * are stale.  Control registers do not appear in the guest register section
     * of the Xen per-cpu stack, so pass it in directly.
     *
     * @param addr Xen virtual address of the guest regs on the per-cpu stack.
     * @param cr3 CR3 for this VCPU.
     * @return boolean indicating success or failure.
     */
    virtual bool parse_regs_from_stack(const vaddr_t & addr, const maddr_t & cr3) throw ();

    /**
     * Parse register information from Xen's struct vcpu.
     *
     * This means that this particular VCPU was inactive at the time of crash,
     * so register state in the struct vcpu is valid.  The vcpu pointer will
     * already be available from parse_basic.
     *
     * @return boolean indicating success or failure.
     */
    virtual bool parse_regs_from_struct() throw ();

    /**
     * Parse register information from other VCPU
     *
     * @return boolean indicating success or failure.
     */
    virtual bool parse_regs_from_active(const VCPU* active) throw ();

    /**
     * Is this VCPU up?
     * @returns boolean testing flags against Xen's _VCPU_Down
     */
    virtual bool is_up() const throw ();

    /**
     * Print the information about this vcpu to the provided stream.
     * Information includes (where relevant).
     * - Core registers
     * - Segment registers
     * - Stack dump
     * - Code dump
     * - Stack trace
     *
     * @param stream Stream to write to.
     * @return Number of bytes written to stream.
     */
    virtual int print_state(FILE * stream) const throw ();

    /**
     * Dump Xen structures for this vcpu.
     *
     * @param stream Stream to write to.
     * @return Number of bytes written to stream.
     */
    virtual int dump_structures(FILE * stream) const throw ();

    /**
     * Print the information about this vcpu to the provided stream, if this
     * vcpu is running in 32bit PV Compatibility mode.
     *
     * Information includes (where relevant).
     * - Core registers
     * - Segment registers
     * - Stack dump
     * - Code dump
     * - Stack trace
     *
     * @param stream Stream to write to.
     * @return Number of bytes written to stream.
     */
    virtual int print_state_compat(FILE * stream) const throw ();

protected:

    /**
     * Common register parsing function.
     *
     * @param addr Xen virtual address of the guest regs on the per-cpu stack.
     * @param cr3 CR3 for this VCPU.
     * @return boolean indicating success or failure.
     */
    virtual bool parse_regs(const vaddr_t & addr, const maddr_t & cr3) throw();

    /// Register values
    x86_64regs regs;
};

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
