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

#ifndef __PCPU_HPP__
#define __PCPU_HPP__

/**
 * @file include/abstract/pcpu.hpp
 * @author Andrew Cooper
 */

#include <stddef.h>
#include <cstdio>

#include "abstract/cpu.hpp"
#include "abstract/vcpu.hpp"

/**
 * Physical CPU state, from Xen crash notes, and Xen per-cpu stack information.
 */
class PCPU: public CPU
{
public:
    /// Constructor.
    PCPU():flags(0),processor_id(-1),per_cpu_offset(0),current_vcpu_ptr(0),
           per_cpu_current_vcpu_ptr(0),ctx_from(NULL),ctx_to(NULL),vcpu(NULL),
           online(true),vcpu_state(CTX_UNKNOWN)
        {};

    /// Destructor.
    virtual ~PCPU(){};

    /**
     * Parse a PR_STATUS crash note.
     *
     * @param buff Buffer containing data.
     * @param len Length of the buffer in bytes.
     * @param index Index of the note, for error reporting.
     * @return boolean indicating success or failure.
     */
    virtual bool parse_pr_status(const char * buff, const size_t len, int index) = 0;

    /**
     * Parse a Xen crash core note.
     *
     * @param buff Buffer containing data.
     * @param len Length of the buffer in bytes.
     * @param index Index of the note, for error reporting.
     * @return boolean indicating success or failure.
     */
    virtual bool parse_xen_crash_core(const char * buff, const size_t len, int index) = 0;

    /**
     * Decode extended state, given information obtained from PR_STATUS and
     * Xen crash core notes.
     * @return boolean indicating success or failure.
     */
    virtual bool decode_extended_state() = 0;

    /**
     * Is this PCPU online?
     * @returns boolean
     */
    virtual bool is_online() const = 0;

    /**
     * Translate a virtual address to a physical address using this cpus cr3 value.
     * @param vaddr Virtual address to translate
     * @param maddr Machine address result of translation
     * @param page_end If non-null, variable to be filled with the last virtual address
     */
    virtual void pagetable_walk(const vaddr_t & vaddr, maddr_t & maddr,
                                vaddr_t * page_end) const = 0;

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

    /// Parsing flags.  Will be made up of PCPU::PCPUFlags
    uint32_t flags;
    /// Processor ID.
    int processor_id;
    /// Per processor offset to per-cpu data.
    uint64_t per_cpu_offset;
    /// Current VCPU pointer from Xen per-cpu stack.
    uint64_t current_vcpu_ptr;
    /// Current VCPU pointer from Xen per-cpu area.
    uint64_t per_cpu_current_vcpu_ptr;

    /// VCPU being context-switched from on this PCPU.  Usually NULL.
    VCPU *ctx_from,
    /// VCPU being context-switched to on this PCPU.  Usually NULL.
        *ctx_to,
    /// VCPU active or idle on this PCPU.  Possibly NULL.
        *vcpu;

    /// Is this PCPU online?
    bool online;

    /**
     * Bitmask flags for what information has been decoded for this PCPU.
     * Indicated state from crash notes and Xen's per-cpu stacks.
     */
    enum PCPUFlags
    {
        /// Core registers are available.
        CPU_CORE_STATE = 1<<0,
        /// Control registers are available.
        CPU_EXTD_STATE = 1<<1,
        /// State from Xen's per-cpu stack is available.
        CPU_STACK_STATE = 1<<2
    };

    /// VCPU state on this PCPU at the time of crash.
    enum PCPUCtxState
    {
        /// Unknown runstate.
        CTX_UNKNOWN = 0,
        /// No VCPU.
        CTX_NONE,
        /// PCPU is running the idle vcpu.
        CTX_IDLE,
        /// VCPU was running.
        CTX_RUNNING,
        /// Context switch occurring.
        CTX_SWITCH
    } vcpu_state;

private:
    // @cond
    PCPU(const PCPU &);
    PCPU & operator= (const PCPU &);
    // @endcond
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
