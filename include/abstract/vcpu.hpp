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

#ifndef __VCPU_HPP__
#define __VCPU_HPP__

/**
 * @file include/abstract/vcpu.hpp
 * @author Andrew Cooper
 */

#include "abstract/cpu.hpp"

/**
 * Abstract Base Class for parsing Xen's virtual CPUs.
 *
 * The parsing is a little complicated because of how active VCPUs at the time
 * of crash have their state stored.
 */
class VCPU: public CPU
{
public:
    /// Constructor.
    VCPU():
        vcpu_ptr(0), domain_ptr(0), vcpu_id(-1), domid(-1), processor(0),
        pause_flags(-1), flags(0), runstate(RST_UNKNOWN),
        paging_support(PAGING_UNKNOWN){};

    /// Destructor
    virtual ~VCPU(){};

    /**
     * Translate a virtual address to a physical address using this cpus cr3 value.
     * @param vaddr Virtual address to translate
     * @param maddr Machine address result of translation
     * @param page_end If non-null, variable to be filled with the last virtual address
     */
    virtual void pagetable_walk(const vaddr_t & vaddr, maddr_t & maddr,
                                vaddr_t * page_end = NULL) const = 0;

    /**
     * Parse basic information from a Xen struct vcpu.
     * Pulls non-register information from Xen's struct vcpu, and associated
     * struct domain.
     * @param addr Xen pointer to a struct vcpu.
     * @param cpu CPU with which pagetable lookups can be performed.
     * @return boolean indicating success or failure.
     */
    virtual bool parse_basic(const vaddr_t & addr, const CPU & cpu) = 0;

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
    virtual bool parse_regs_from_stack(const vaddr_t & addr, const maddr_t & cr3) = 0;

    /**
     * Parse register information from Xen's struct vcpu.
     *
     * This means that this particular VCPU was inactive at the time of crash,
     * so register state in the struct vcpu is valid.  The vcpu pointer will
     * already be available from parse_basic.
     *
     * @return boolean indicating success or failure.
     */
    virtual bool parse_regs_from_struct() = 0;

    /**
     * Parse register information from other VCPU
     *
     * @param active Active VCPU with already-parsed registers.
     * @return boolean indicating success or failure.
     */
    virtual bool parse_regs_from_active(const VCPU* active)= 0;

    /**
     * Is this VCPU online?
     * @returns boolean testing flags against Xen's _VCPU_Down
     */
    virtual bool is_online() const = 0;

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
    virtual int print_state(FILE * stream) const = 0;

    /**
     * Dump Xen structures for this vcpu.
     *
     * @param stream Stream to write to.
     * @return Number of bytes written to stream.
     */
    virtual int dump_structures(FILE * stream) const = 0;

    /// Xen pointer to this struct vcpu.
    vaddr_t vcpu_ptr;
    /// Xen pointer to the struct domain, to which this vcpu belongs.  From struct vcpu.
    vaddr_t domain_ptr;
    /// Vcpu ID, from struct vcpu.
    uint32_t vcpu_id;
    /// Domain ID, from struct domain.
    uint16_t domid;
    /// processor number, from struct vcpu.
    uint32_t processor;
    /// Vcpu pause flags, from struct vcpu.
    uint32_t pause_flags;

    /// Parsing flags.  Will be made up of VCPU::VCPUFlags.
    uint32_t flags;

    /**
     * Bitmask flags for what information has been decoded for this VCPU.
     * Includes state pulled from Xen's struct vcpu, the per-cpu stacks, and the
     * associated struct domain.
     */
    enum VCPUFlags
    {
        /// Core registers are available.
        CPU_CORE_STATE = 1<<0,
        /// Control registers are available.
        CPU_EXTD_STATE = 1<<1,

        /// VCPU is running in PV Compatibility mode (a.k.a. 32bit mode on 64bit Xen)
        CPU_PV_COMPAT = 1<<2,
        /// VCPU is an HVM VCPU
        CPU_HVM = 1<<3
    };

    /// Runstate of this VCPU at the time of crash
    enum VCPURunstate
    {
        /// Unknown runstate.
        RST_UNKNOWN = 0,
        /// VCPU was not running
        RST_NONE,
        /// VCPU was running.
        RST_RUNNING,
        /// VCPU was being context switched from.
        RST_CTX_SWITCH
    } runstate;

    /// Paging support which this VCPU has from Xen.
    enum VCPUPagingSupport
    {
        /// Unknown support.
        PAGING_UNKNOWN,
        /// No paging support.
        PAGING_NONE,
        /// Shadow paging support.
        PAGING_SHADOW,
        /// Hardware Assisted Paging support.
        PAGING_HAP
    } paging_support;

};

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
