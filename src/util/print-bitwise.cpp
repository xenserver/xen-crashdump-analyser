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

/**
 * @file src/util/print-bitwise.cpp
 * @author Andrew Cooper
 */

#include "util/print-bitwise.hpp"

#include "util/stdio-wrapper.hpp"

/**
 * Macro to help with bitwise decoding of registers.
 * @param b Bit number of the register.
 * @param n Symbolic name of the specified bit.
 */
#define BIT(b, n) do { if (reg & (1<<(b))) len+=FPUTS(" "#n, o); } while(0)

int print_cr0(FILE * o, const uint64_t & reg)
{
    int len = 0;

    BIT(31, PG); // Paging
    BIT(30, CD); // Cache Disable
    BIT(29, NW); // Not Write-through
    BIT(18, AM); // Alignment Mask
    BIT(16, WP); // Write Protect
    BIT(5 , NE); // Numeric Error
    BIT(4 , ET); // Extension Type
    BIT(3 , TS); // Task Switched
    BIT(2 , EM); // Emulation
    BIT(1 , MP); // Monitor Coprocessor
    BIT(0 , PE); // Protection Enable

    return len;
}

int print_cr4(FILE * o, const uint64_t & reg)
{
    int len = 0;

    BIT(20, SMEP); // Supervisor Mode Execution Disable
    BIT(18, OSXSAVE); // OS XSAVE support
    BIT(17, PCIDE); // Process-context IDs
    BIT(16, FSGSBASE);

    BIT(14, SMXE); // SMX Enable
    BIT(13, VMXE); // VMX Enable

    BIT(10, OSXMMEXCPT);
    BIT(9 , OSFXSR); // OS support for FX{SAVE,RSTOR}
    BIT(8 , PCE); // Performance Counter Enable
    BIT(7 , PGE); // Page Global Enable
    BIT(6 , MCE); // Machine Check Enable
    BIT(5 , PAE); // Page Address Extensions
    BIT(4 , PSE); // Page Size Extensions
    BIT(3 , DE); // Debugging Extensions
    BIT(2 , TSD); // Time Stamp Disable
    BIT(1 , PVI); // Protected-Mode Virtual Interrupts
    BIT(0 , VME); // Virtual 8086 Extensions

    return len;
}

int print_rflags(FILE * o, const uint64_t & reg)
{
    int len = 0;

    BIT(21, ID); // Identification
    BIT(20, VIP); // Virtual Interrupt Pending
    BIT(19, VIF); // Virtual Interrupt
    BIT(18, AC); // Alignment Check
    BIT(17, VM); // Virtual 8086 mode
    BIT(16, RF); // Resume

    BIT(14, NT); // Nested Task
    len += FPRINTF(o, " IOPL%"PRIu64, reg & (3<<12));
    BIT(8 , TF); // Trap

    len += FPUTS("   ", o);

    BIT(11, OF); // Overflow flag
    BIT(10, DF); // Direction flag
    BIT(9 , IF); // Interrupt Enable
    BIT(7 , SF); // Sign Flag
    BIT(6 , ZF); // Zero Flag
    BIT(4 , AF); // Adjust Flag
    BIT(2 , PF); // Parity Flag
    BIT(0 , CF); // Carry Flag

    return len;
}

int print_pause_flags(FILE * o, const uint32_t & reg)
{
    int len = 0;

    BIT(4, Mem_Event);
    BIT(3, Migrating);
    BIT(2, Blocked_in_Xen);
    BIT(1, Down);
    BIT(0, Blocked);

    return len;
}

int print_paging_mode(FILE * o, const uint32_t & reg)
{
    int len = 0;

    if ( reg == 0 )
        return len + FPUTS("None", o);

    BIT(21, HAP);
    BIT(20, Shadow);
    BIT(14, external);
    BIT(13, translate);
    BIT(12, log_dirty);
    BIT(11, refcounts);

    return len;
}

#undef BIT

/*
 * Local variables:
 * mode: C++
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
