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
 *  Copyright (c) 2011,2012 Citrix Inc.
 */

#include "symbols.hpp"
#include "util/log.hpp"

/**
 * @file src/symbols.cpp
 * @author Andrew Cooper
 */

/**
 * Macro for declaring a group of related symbols.
 *
 * It sets up the public symbol 'required_\<group\>_symbols', and a private
 * variable used by SYMBOL() to set up the relevant masks.
 *
 * The SYMBOL()'s (ab)use of mask and the comma operator will work for up to
 * 63 symbols per group, which is sufficent for now.
 *
 * @param g Symbol group name.
 */
#define GROUP(g) uint64_t required_##g##_symbols; static uint64_t _meta_##g##_sym_mask = 1

vaddr_t conring, conring_size;
/// Required console symbols
GROUP(console);

vaddr_t conringp, conringc;
/// Required console producer/consumer symbols
GROUP(consolepc);

vaddr_t CPUINFO_sizeof, CPUINFO_processor_id, CPUINFO_current_vcpu,
    CPUINFO_per_cpu_offset, CPUINFO_guest_cpu_user_regs;
/// Required cpuinfo symbols
GROUP(cpuinfo);

vaddr_t VCPU_sizeof, VCPU_vcpu_id, VCPU_processor, VCPU_pause_flags,
    VCPU_thread_flags, VCPU_user_regs, VCPU_cr3, VCPU_domain;
/// Required vcpu symbols
GROUP(vcpu);

vaddr_t DOMAIN_id, DOMAIN_is_32bit_pv, DOMAIN_is_hvm, DOMAIN_is_privileged,
    DOMAIN_max_vcpus, DOMAIN_vcpus, DOMAIN_next, DOMAIN_tot_pages, DOMAIN_max_pages,
    DOMAIN_shr_pages, DOMAIN_handle, DOMAIN_paging_mode ,DOMAIN_sizeof, domain_list;
/// Required domain symbols
GROUP(domain);

vaddr_t __per_cpu_offset, idle_vcpu;
vaddr_t per_cpu__curr_vcpu;
/// Required per_cpu symbols
GROUP(per_cpu);


#undef GROUP

/**
 * Macro to help setting up the required symbols table.
 *
 * It (ab)uses the comma operator and the two variables from the GROUP() macro
 * to count the total number of symbols, and create bitmasks for tracking
 * which have been found in the symbol table.
 *
 * @param g Symbol group.
 * @param n Symbol Index.
 */
#define SYMBOL(g, n) { #n , &(n) , &(required_##g##_symbols) , \
        (_meta_##g##_sym_mask <<= 1, required_##g##_symbols = _meta_##g##_sym_mask - 1, \
         _meta_##g##_sym_mask >> 1) }

/// Required symbols container
const static struct required_symbol
{
    /// Name.
    const char * name;
    /// Address.
    vaddr_t * address;
    /// Symbol group.
    uint64_t * group;
    /// Found mask.
    uint64_t mask;
}
required_symbols [] =
{
    SYMBOL(console, conring),
    SYMBOL(console, conring_size),

    SYMBOL(consolepc, conringp),
    SYMBOL(consolepc, conringc),

    SYMBOL(cpuinfo, CPUINFO_sizeof),
    SYMBOL(cpuinfo, CPUINFO_processor_id),
    SYMBOL(cpuinfo, CPUINFO_current_vcpu),
    SYMBOL(cpuinfo, CPUINFO_per_cpu_offset),
    SYMBOL(cpuinfo, CPUINFO_guest_cpu_user_regs),

    SYMBOL(vcpu, VCPU_sizeof),
    SYMBOL(vcpu, VCPU_vcpu_id),
    SYMBOL(vcpu, VCPU_processor),
    SYMBOL(vcpu, VCPU_pause_flags),
    SYMBOL(vcpu, VCPU_thread_flags),
    SYMBOL(vcpu, VCPU_user_regs),
    SYMBOL(vcpu, VCPU_cr3),
    SYMBOL(vcpu, VCPU_domain),

    SYMBOL(domain, DOMAIN_id),
    SYMBOL(domain, DOMAIN_is_32bit_pv),
    SYMBOL(domain, DOMAIN_is_hvm),
    SYMBOL(domain, DOMAIN_is_privileged),
    SYMBOL(domain, DOMAIN_max_vcpus),
    SYMBOL(domain, DOMAIN_vcpus),
    SYMBOL(domain, DOMAIN_next),
    SYMBOL(domain, DOMAIN_tot_pages),
    SYMBOL(domain, DOMAIN_max_pages),
    SYMBOL(domain, DOMAIN_shr_pages),
    SYMBOL(domain, DOMAIN_handle),
    SYMBOL(domain, DOMAIN_paging_mode),
    SYMBOL(domain, DOMAIN_sizeof),
    SYMBOL(domain, domain_list),

    SYMBOL(per_cpu, __per_cpu_offset),
    SYMBOL(per_cpu, idle_vcpu),
    SYMBOL(per_cpu, per_cpu__curr_vcpu),

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
