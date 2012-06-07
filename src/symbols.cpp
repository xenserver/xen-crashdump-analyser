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

#include "symbols.hpp"
#include "util/log.hpp"

/**
 * @file src/symbols.cpp
 * @author Andrew Cooper
 */

/**
 * Macro to help setting up the required bitmasks.
 * @param g Symbol group.
 * @param c Count of symbols in this group.
 */
#define REQUIRE(g, c) int required_##g##_symbols = ((1<<(c))-1)

vaddr_t conring, conring_size;
/// Required console symbols
REQUIRE(console, 2);

vaddr_t conringp, conringc;
/// Required console producer/consumer symbols
REQUIRE(consolepc, 2);

vaddr_t CPUINFO_sizeof, CPUINFO_processor_id, CPUINFO_current_vcpu,
    CPUINFO_per_cpu_offset, CPUINFO_guest_cpu_user_regs;
/// Required cpuinfo symbols
REQUIRE(cpuinfo, 5);

vaddr_t VCPU_sizeof, VCPU_vcpu_id, VCPU_processor, VCPU_pause_flags,
    VCPU_thread_flags, VCPU_user_regs, VCPU_cr3, VCPU_domain;
/// Required vcpu symbols
REQUIRE(vcpu, 8);

vaddr_t DOMAIN_id, DOMAIN_is_32bit_pv, DOMAIN_is_hvm, DOMAIN_is_privileged,
    DOMAIN_max_vcpus, DOMAIN_vcpus, DOMAIN_next, DOMAIN_tot_pages, DOMAIN_max_pages,
    DOMAIN_shr_pages, domain_list;
/// Required domain symbols
REQUIRE(domain, 11);

vaddr_t __per_cpu_offset, idle_vcpu;
vaddr_t per_cpu__curr_vcpu;
/// Required per_cpu symbols
REQUIRE(per_cpu, 3);


#undef REQUIRE

/**
 * Macro to help setting up the required symbols table.
 * @param g Symbol group.
 * @param i Symbol Index.
 * @param n Symbol name.
 */
#define SYMBOL(g, i, n) { #n , &(n) , &(required_##g##_symbols) , 1<<((i)-1) }

/// Required symbols container
const static struct required_symbol
{
    /// Name.
    const char * name;
    /// Address.
    vaddr_t * address;
    /// Symbol group.
    int * group;
    /// Found mask.
    int mask;
}
required_symbols [] =
{
    SYMBOL(console, 1, conring),
    SYMBOL(console, 2, conring_size),

    SYMBOL(consolepc, 1, conringp),
    SYMBOL(consolepc, 2, conringc),

    SYMBOL(cpuinfo, 1, CPUINFO_sizeof),
    SYMBOL(cpuinfo, 2, CPUINFO_processor_id),
    SYMBOL(cpuinfo, 3, CPUINFO_current_vcpu),
    SYMBOL(cpuinfo, 4, CPUINFO_per_cpu_offset),
    SYMBOL(cpuinfo, 5, CPUINFO_guest_cpu_user_regs),

    SYMBOL(vcpu, 1, VCPU_sizeof),
    SYMBOL(vcpu, 2, VCPU_vcpu_id),
    SYMBOL(vcpu, 3, VCPU_processor),
    SYMBOL(vcpu, 4, VCPU_pause_flags),
    SYMBOL(vcpu, 5, VCPU_thread_flags),
    SYMBOL(vcpu, 6, VCPU_user_regs),
    SYMBOL(vcpu, 7, VCPU_cr3),
    SYMBOL(vcpu, 8, VCPU_domain),

    SYMBOL(domain,  1, DOMAIN_id),
    SYMBOL(domain,  2, DOMAIN_is_32bit_pv),
    SYMBOL(domain,  3, DOMAIN_is_hvm),
    SYMBOL(domain,  4, DOMAIN_is_privileged),
    SYMBOL(domain,  5, DOMAIN_max_vcpus),
    SYMBOL(domain,  6, DOMAIN_vcpus),
    SYMBOL(domain,  7, DOMAIN_next),
    SYMBOL(domain,  8, DOMAIN_tot_pages),
    SYMBOL(domain,  9, DOMAIN_max_pages),
    SYMBOL(domain, 10, DOMAIN_shr_pages),
    SYMBOL(domain, 11, domain_list),

    SYMBOL(per_cpu, 1, __per_cpu_offset),
    SYMBOL(per_cpu, 2, idle_vcpu),
    SYMBOL(per_cpu, 3, per_cpu__curr_vcpu),

#undef SYMBOL
    // Terminating SYMBOL for loop in insert_required_symbol()
    { NULL, NULL, NULL, 0 }
};

void insert_required_symbol(const char * name, vaddr_t & address)
{
    const struct required_symbol * rc;

    for ( rc = &required_symbols[0]; rc->name; ++rc )
    {
        if ( std::strcmp(name, rc->name) != 0 )
            continue;

        if ( ! ((*rc->group) & rc->mask) )
        {
            LOG_INFO("Discarding duplicate symbol %s\n", name);
            break;
        }

        (*rc->address) = address;
        (*rc->group) &= ~rc->mask;

        break;
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
