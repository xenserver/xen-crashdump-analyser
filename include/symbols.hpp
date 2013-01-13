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

#ifndef __SYMBOLS_HPP__
#define __SYMBOLS_HPP__

/**
 * @file include/symbols.hpp
 * @author Andrew Cooper
 */

#include "types.hpp"
#include <cstring>

/// Xen conring symbol, from symbol table.  Pointer to console ring.
extern vaddr_t conring;
/// Xen conring_size symbol, from symbol table.  Length of console ring.
extern vaddr_t conring_size;
/// Bitmask of Xen's console related symbols from the symbol table.
extern uint64_t required_console_symbols;

/// Xen conringp symbol, from symbol table.  Console producer index.
extern vaddr_t conringp;
/// Xen conringc symbol, from symbol table  Console consumer index.
extern vaddr_t conringc;
/// Bitmask of Xen's console index related symbols from the symbol table.
extern uint64_t required_consolepc_symbols;

/// Sizeof Xen's cpuinfo structure.
extern vaddr_t CPUINFO_sizeof;
/// Offset of processor_id in Xen's struct cpuinfo.
extern vaddr_t CPUINFO_processor_id;
/// Offset of current_vcpu in Xen's struct cpuinfo.
extern vaddr_t CPUINFO_current_vcpu;
/// Offset of per_cpu_offset in Xen's struct cpuinfo.
extern vaddr_t CPUINFO_per_cpu_offset;
/// Offset of guest_cpu_user_regs in Xen's struct cpuinfo.
extern vaddr_t CPUINFO_guest_cpu_user_regs;
/// Bitmask of Xen's struct cpuinfo related offsets from the symbol table.
extern uint64_t required_cpuinfo_symbols;

/// Sizeof Xen's vcpu structure.
extern vaddr_t VCPU_sizeof;
/// Offset of vcpu_id in Xen's struct vcpu.
extern vaddr_t VCPU_vcpu_id;
/// Offset of processor in Xen's struct vcpu.
extern vaddr_t VCPU_processor;
/// Offset of pause_flags in Xen's struct vcpu.
extern vaddr_t VCPU_pause_flags;
/// Offset of thread_flags in Xen's struct vcpu.
extern vaddr_t VCPU_thread_flags;
/// Offset of user_regs in Xen's struct vcpu.
extern vaddr_t VCPU_user_regs;
/// Offset of cr3 in Xen's struct vcpu.
extern vaddr_t VCPU_cr3;
/// Offset of domain in Xen's struct vcpu.
extern vaddr_t VCPU_domain;
/// Bitmask of Xen's struct vcpu related offsets from the symbol table.
extern uint64_t required_vcpu_symbols;

/// Offset of id in Xen's struct domain.
extern vaddr_t DOMAIN_id;
/// Offset of is_32bit_pv in Xen's struct domain.
extern vaddr_t DOMAIN_is_32bit_pv;
/// Offset of is_hvm in Xen's struct domain.
extern vaddr_t DOMAIN_is_hvm;
/// Offset of is_privileged in Xen's struct domain.
extern vaddr_t DOMAIN_is_privileged;
/// Offset of max_vcpus in Xen's struct domain.
extern vaddr_t DOMAIN_max_vcpus;
/// Offset of vcpus in Xen's struct domain.
extern vaddr_t DOMAIN_vcpus;
/// Offset of next in Xen's struct domain.
extern vaddr_t DOMAIN_next;
/// Offset of tot_pages in Xen's struct domain.
extern vaddr_t DOMAIN_tot_pages;
/// Offset of max_pages in Xen's struct domain.
extern vaddr_t DOMAIN_max_pages;
/// Offset of shr_pages in Xen's struct domain.
extern vaddr_t DOMAIN_shr_pages;
/// Offset of handle in Xen's struct domain.
extern vaddr_t DOMAIN_handle;
/// Offset of arch.paging.mode in Xen's struct domain.
extern vaddr_t DOMAIN_paging_mode;
/// Size of Xen's struct domain.
extern uint64_t DOMAIN_sizeof;
/// Xen's domain_list symbol.
extern vaddr_t domain_list;
/// Bitmask of Xen's struct domain related offsets from the symbol table.
extern uint64_t required_domain_symbols;

/// Xen's per_cpu__curr_vcpu symbol.
extern vaddr_t per_cpu__curr_vcpu;
/// Xen's idle_vcpu symbol.
extern vaddr_t idle_vcpu;
/// Xen's __per_cpu_offset symbol
extern vaddr_t __per_cpu_offset;
/// Bitmask of Xen's per_cpu related symbols from the symbol table.
extern uint64_t required_per_cpu_symbols;

/**
 * Insert a symbol or offset into the required table.
 *
 * @param name Symbol or offset name
 * @param address Address or value of symbol or offset
 */
void insert_required_symbol(const char * name, vaddr_t & address);

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
