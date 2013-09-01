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

#include "types.hpp"
#include "util/macros.hpp"
#include "abstract/pagetable.hpp"

namespace Abstract
{

/**
 * Abstract Base Class for parsing Xen's virtual CPUs.
 *
 * The parsing is a little complicated because of how active VCPUs at the time
 * of crash have their state stored.
 */
    class VCPU
    {
    public:

        /// VCPU Runstate.
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
        };

        /**
         * Constructor.
         * @param rst VCPU Runstate.
         */
        VCPU(VCPURunstate rst):
            vcpu_ptr(0), domain_ptr(0), vcpu_id(-1), domid(-1), processor(0),
            pause_flags(-1), pause_count(-1), flags(0), dompt(NULL),
            runstate(rst), paging_support(PAGING_UNKNOWN){};

        /// Destructor.
        virtual ~VCPU()
        {
            SAFE_DELETE(this->dompt);
        };

        /**
         * Parse basic information from a Xen struct vcpu.
         * Pulls non-register information from Xen's struct vcpu, and associated
         * struct domain.
         * @param addr Xen pointer to a struct vcpu.
         * @param xenpt PageTable with which translations can be performed.
         * @return boolean indicating success or failure.
         */
        virtual bool parse_basic(const vaddr_t & addr, const Abstract::PageTable & xenpt) = 0;

        /**
         * Parse extended VCPU information, including registers.
         *
         * The vcpu rustate should have already been set, so
         * parse_extended() can work out exactly where to get its
         * register information from.
         *
         * @param xenpt PageTable with which translations can be performed.
         * @param cpuinfo Optional pointer to a xen address of the per-pcpu
         * stack cpuinfo block.  This is only relevent for running vcpus at
         * the time of crash.
         * @return boolean indicating success or failure.
         */
        virtual bool parse_extended(const Abstract::PageTable & xenpt,
                                    const vaddr_t * cpuinfo = NULL ) = 0;

        /**
         * Copy VCPU state from active vcpu
         *
         * @param active An active VCPU which already has its complete state.
         * @return boolean indicating success or failure.
         */
        virtual bool copy_from_active(const VCPU* active) = 0;

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
         * @param xenpt PageTable with which translations can be performed.
         * @return Number of bytes written to stream.
         */
        virtual int dump_structures(FILE * stream, const Abstract::PageTable & xenpt) const = 0;

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
        /// Vcpu pause count, from struct vcpu.
        uint32_t pause_count;

        /// Parsing flags.  Will be made up of VCPU::VCPUFlags.
        uint32_t flags;

        /// Pagetable for the domain context of this VCPU.
        Abstract::PageTable * dompt;

        /**
         * Bitmask flags for what information has been decoded for this VCPU.
         * Includes state pulled from Xen's struct vcpu, the per-cpu stacks, and the
         * associated struct domain.
         */
        enum VCPUFlags
        {
            /// Core registers are available.
            CPU_GP_REGS = 1<<0,
            /// Segment registers are available.
            CPU_SEG_REGS = 1<<1,
            /// Control registers are available.
            CPU_CR_REGS = 1<<2,

            /// VCPU is running in PV Compatibility mode (a.k.a. 32bit mode on 64bit Xen)
            CPU_PV_COMPAT = 1<<3,
            /// VCPU is an HVM VCPU
            CPU_HVM = 1<<4
        };

        /// Runstate of this VCPU at the time of crash.
        VCPURunstate runstate;

        /// VCPU Paging mode.
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
        }
        /// Paging support which this VCPU has from Xen.
            paging_support;

    private:
        // @cond EXCLUDE
        VCPU(const VCPU &);
        VCPU & operator= (const VCPU &);
        // @endcond

    };

}

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
