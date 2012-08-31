/*
 *  This file is part of the Xen Crashdump Analyser.
 *
 *  Foobar is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Foobar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2011,2012 Citrix Inc.
 */

#include "system.hpp"
#include "util/log.hpp"

#include <cstring>

/**
 * @file src/system.cpp
 * @author Andrew Cooper
 */

enum CPU_VENDOR cpu_vendor = VENDOR_UNKNOWN;

/// Intel cpuid string identifier.
const static char vendor_string_intel[] = "GenuineIntel";
/// AMD cpuid string identifier.
const static char vendor_string_amd[] = "AuthenticAMD";

uint8_t maxphysaddr = 0;
uint64_t physaddrmask = 0;

/**
 * Execute the cpuid instruction, with the provided register state.
 * @param eax eax reference.
 * @param ebx ebx reference.
 * @param ecx ecx reference.
 * @param edx edx reference.
 * @return values through the parameter references.
 */
static void cpuid(uint32_t & eax, uint32_t & ebx, uint32_t & ecx, uint32_t & edx)
{
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx):
                  "a"(eax), "b"(ebx), "c"(ecx), "d"(edx) );
}

void gather_system_information()
{
    uint32_t eax, ebx, ecx, edx;
    union { uint32_t regs[3]; char name[12]; } vendor_string = { {0, 0, 0} };

    /* Check the vendor string.  This is needed to distinguish between EPT or NPT
     * when trying to read guests address spaces with HAP enabled .*/
    eax = 0;
    cpuid(eax, vendor_string.regs[0], vendor_string.regs[2], vendor_string.regs[1]);

    if ( !strncmp(vendor_string.name, vendor_string_intel, sizeof vendor_string.name) )
    {
        LOG_INFO("CPU vendor is Intel\n");
        cpu_vendor = VENDOR_INTEL;
    }
    else if ( !strncmp(vendor_string.name, vendor_string_amd, sizeof vendor_string.name) )
    {
        LOG_INFO("CPU vendor is AMD\n");
        cpu_vendor = VENDOR_AMD;
    }
    else
    {
        LOG_INFO("CPU vendor is unknown\n");
        cpu_vendor = VENDOR_UNKNOWN;
    }

    /* Collect maxphysaddr which affects the valid bits in pagetable entries. */
    ebx = ecx = edx = 0;
    eax = 0x80000000;
    cpuid(eax, ebx, ecx, edx);

    if ( eax < 0x80000008 )
        LOG_WARN("Failed to find maxphysaddr\n");
    else
    {
        eax = 0x80000008;
        cpuid(eax, ebx, ecx, edx);
        maxphysaddr = ( eax & 0xff );
        physaddrmask = ((1ULL << maxphysaddr)-1);
        LOG_DEBUG("maxphysaddr = %"PRIu8", mask = 0x%016"PRIx64"\n",
                  maxphysaddr, physaddrmask);
    }
}


/*
 * Local variables:
 * mode: C++
 * c-set-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
