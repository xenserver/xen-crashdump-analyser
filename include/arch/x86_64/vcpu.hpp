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

namespace x86_64
{

/**
 * Class for parsing a 64bit Xen's virtual CPUs.
 *
 * The parsing is a little complicated because of how active VCPUs at the time
 * of crash have their state stored.
 */
    class VCPU : public Abstract::VCPU
    {
    public:
        /**
         * Constructor.
         * @param rst VCPU Runstate.
         */
        VCPU(Abstract::VCPU::VCPURunstate rst);

        /// Destructor.
        virtual ~VCPU();

        /**
         * Parse basic information from a Xen struct vcpu.
         * Pulls non-register information from Xen's struct vcpu, and associated
         * struct domain.
         * @param addr Xen pointer to a struct vcpu.
         * @param xenpt PageTable with which pagetable lookups can be performed.
         * @return boolean indicating success or failure.
         */
        virtual bool parse_basic(const vaddr_t & addr, const Abstract::PageTable & xenpt);

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
                                    const vaddr_t * cpuinfo = NULL);

        /**
         * Copy VCPU state from active vcpu.
         *
         * @param active An active VCPU which already has its complete state.
         * @return boolean indicating success or failure.
         */
        virtual bool copy_from_active(const Abstract::VCPU* active);

        /**
         * Is this VCPU online?
         * @returns boolean testing flags against Xen's _VCPU_Down
         */
        virtual bool is_online() const;

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
        virtual int print_state(FILE * stream) const;

        /**
         * Dump Xen structures for this vcpu.
         *
         * @param stream Stream to write to.
         * @param xenpt PageTable with which translations can be performed.
         * @return Number of bytes written to stream.
         */
        virtual int dump_structures(FILE * stream, const Abstract::PageTable & xenpt) const;

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
        virtual int print_state_compat(FILE * stream) const;

    protected:

        /**
         * Common register parsing function.
         *
         * @param addr Xen virtual address of the guest regs.
         * @param xenpt PageTable with which translations can be performed.
         * @return boolean indicating success or failure.
         */
        virtual bool parse_regs(const vaddr_t & addr, const Abstract::PageTable & xenpt);

        /// Register values
        x86_64regs regs;
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
