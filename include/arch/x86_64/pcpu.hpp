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

/**
 * Physical CPU state, from Xen crash notes, and Xen per-cpu stack information
 * for 64bit Xen
 */
class x86_64PCPU : public PCPU
{
public:
    /// Constructor.
    x86_64PCPU();

    /// Destructor.
    virtual ~x86_64PCPU();

    /**
     * Parse a PR_STATUS crash note.
     *
     * @param buff Buffer containing data.
     * @param len Length of the buffer in bytes.
     */
    virtual bool parse_pr_status(const char * buff, const size_t len) throw ();

    /**
     * Parse a Xen crash core note.
     *
     * @param buff Buffer containing data.
     * @param len Length of the buffer in bytes.
     */
    virtual bool parse_xen_crash_core(const char * buff, const size_t len) throw ();

    /**
     * Decode extended state, given information obtained from PR_STATUS and
     * Xen crash core notes.
     */
    virtual bool decode_extended_state() throw ();

    /**
     * Translate a virtual address to a physical address using this cpus cr3 value.
     * @param vaddr Virtual address to translate
     * @param maddr Machine address result of translation
     * @param page_end If non-null, variable to be filled with the last virtual address
     */
    virtual void pagetable_walk(const vaddr_t & vaddr, maddr_t & maddr,
                                vaddr_t * page_end) const;

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

protected:
    /// PCPU Registers
    x86_64regs regs;

    /**
     * Print xen per-cpu stack.
     *
     * Include extending parsing of interrupt stack tables.
     * @param stream Stream to write to.
     * @param stack Xen's per-cpu stack pointer.
     */
    int print_stack(FILE * stream, const vaddr_t & stack) const throw ();

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
