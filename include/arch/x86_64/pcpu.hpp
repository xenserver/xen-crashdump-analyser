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

#ifndef __X86_64_PCPU_HPP__
#define __X86_64_PCPU_HPP__

/**
 * @file include/arch/x86_64/pcpu.hpp
 * @author Andrew Cooper
 */

#include "abstract/pcpu.hpp"
#include "arch/x86_64/structures.hpp"

namespace x86_64
{

/**
 * Physical CPU state, from Xen crash notes, and Xen per-cpu stack information
 * for 64bit Xen
 */
    class PCPU : public Abstract::PCPU
    {
    public:
        /// Constructor.
        PCPU();

        /// Destructor.
        virtual ~PCPU();

        /**
         * Parse a PR_STATUS crash note.
         *
         * @param buff Buffer containing data.
         * @param len Length of the buffer in bytes.
         * @param index Index of the note, for error reporting.
         * @return boolean indicating success or failure.
         */
        virtual bool parse_pr_status(const char * buff, const size_t len, int index);

        /**
         * Parse a Xen crash core note.
         *
         * @param buff Buffer containing data.
         * @param len Length of the buffer in bytes.
         * @param index Index of the note, for error reporting.
         * @return boolean indicating success or failure.
         */
        virtual bool parse_xen_crash_core(const char * buff, const size_t len, int index);

        /**
         * Probe a stack looking for Xen information.
         *
         * @param cr3 Any cr3 which can viably used to parse Xen state
         * @param stack_base Base of the area suspected to be a Xen stack
         * @return boolean indicating success or failure.
         */
        virtual bool probe_xen_stack(uint64_t cr3, vaddr_t stack_base);

        /**
         * Decode extended state, given information obtained from PR_STATUS and
         * Xen crash core notes.
         * @return boolean indicating success or failure.
         */
        virtual bool decode_extended_state();

        /**
         * Is this PCPU online?
         * @returns boolean
         */
        virtual bool is_online() const;

        /**
         * Try to declare the PCPU online, if sufficient state is available.
         */
        virtual void try_online();

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
         * Dump entire stack contents.
         *
         * Designed for power users to interpret
         *
         * @param stream Stream to write to.
         * @return Number of bytes written to stream.
         */
        virtual int dump_stack(FILE * stream) const;

    protected:
        /// PCPU Registers
        x86_64regs regs;

        /**
         * Print xen per-cpu stack.
         *
         * Include extending parsing of interrupt stack tables.
         * @param stream Stream to write to.
         * @param stack Xen's per-cpu stack pointer.
         * @param mask Bitmask of visited stack pages to avoid unbounded recursion.
         * @return Number of bytes written to stream.
         */
        int print_stack(FILE * stream, const vaddr_t & stack, unsigned mask) const;

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
