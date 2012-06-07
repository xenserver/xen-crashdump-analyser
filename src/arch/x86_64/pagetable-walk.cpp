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

/**
 * @file src/arch/x86_64/pagetable-walk.cpp
 * @author Andrew Cooper
 */

#include "arch/x86_64/pagetable-walk.hpp"

#include "util/log.hpp"
#include "memory.hpp"

/// Is the present bit set for a pagetable entry
#define present(v)     ((v) & 1)
/// Is the page size bit set for a pagetable entry
#define page_size(v)   ((v) & (1<<7))

/// Calculate entry offset into the PM4L based on a virtual address
#define pm4l_offset(v) (((v >> 39) & ((1<<9)-1)) * 8)
/// Calculate entry offset into the PDPT based on a virtual address
#define pdpt_offset(v) (((v >> 30) & ((1<<9)-1)) * 8)
/// Calculate entry offset into the PD based on a virtual address
#define pd_offset(v)   (((v >> 21) & ((1<<9)-1)) * 8)
/// Calculate entry offset into the PT based on a virtual address
#define pt_offset(v)   (((v >> 12) & ((1<<9)-1)) * 8)

/// Calculate offset into a 4K page given a base physical address and virtual address
#define offset_4K(b, v)   ((b) | ((v) & ((1ULL<<12)-1)))
/// Calculate offset into a 2M superpage given a base physical address and virtual address
#define offset_2M(b, v)   ((b) | ((v) & ((1ULL<<21)-1)))
/// Calculate offset into a 1G superpage given a base physical address and virtual address
#define offset_1G(b, v)   ((b) | ((v) & ((1ULL<<30)-1)))
/// Calculate offset into a 512G superpage given a base physical address and virtual address
#define offset_512G(b, v) ((b) | ((v) & ((1ULL<<39)-1)))

/// Round an address up to the last byte in a 4K page
#define roundup_4K(v)   ((v) | ((1ULL<<12)-1))
/// Round an address up to the last byte in a 2M superpage
#define roundup_2M(v)   ((v) | ((1ULL<<21)-1))
/// Round an address up to the last byte in a 1G superpage
#define roundup_1G(v)   ((v) | ((1ULL<<30)-1))
/// Round an address up to the last byte in a 512G superpage
#define roundup_512G(v) ((v) | ((1ULL<<39)-1))

void pagetable_walk_64(const maddr_t & cr3, const vaddr_t & vaddr,
                       maddr_t & maddr, vaddr_t * page_end)
{
    // cr3 has the pml4 physical address between bits 51 and 12
    // each page entry contain the next physical address between the same bits
    static const uint64_t addr_mask = 0x000FFFFFFFFFF000ULL;

    maddr_t pml4_entry;

    maddr_t pdpt_base;
    maddr_t pdpt_entry;

    maddr_t pd_base;
    maddr_t pd_entry;

    maddr_t pt_base;
    maddr_t pt_entry;

    maddr_t page;

    /* While this could technically be valid under x86 architecture, it is
     * certainly invalid under a sensible Xen setup, and implies a failure to
     * parse a {P,V}CPU correctly.
     */
    if ( ! cr3 )
        throw pagefault(vaddr, cr3, 5);

    memory.read64((cr3 & addr_mask) + pm4l_offset(vaddr),
                  pml4_entry);

    // PDPT present?
    if ( ! present(pml4_entry) )
        throw pagefault(vaddr, cr3, 4);

    pdpt_base = pml4_entry & addr_mask;

    // Page Size bit set? (512G superpage)
    if ( page_size(pml4_entry) )
    {
        maddr = offset_512G(pdpt_base, vaddr);
        if ( page_end )
            *page_end = roundup_512G(vaddr);
        return;
    }

    memory.read64(pdpt_base + pdpt_offset(vaddr),
                  pdpt_entry);

    // PD present?
    if ( ! present(pdpt_entry) )
        throw pagefault(vaddr, cr3, 3);

    pd_base = pdpt_entry & addr_mask;

    // Page Size bit set? (1G superpage)
    if ( page_size(pdpt_entry) )
    {
        maddr = offset_1G(pd_base, vaddr);
        if ( page_end )
            *page_end = roundup_1G(vaddr);
        return;
    }

    memory.read64(pd_base + pd_offset(vaddr),
                  pd_entry);

    // PT present?
    if ( ! present(pd_entry) )
        throw pagefault(vaddr, cr3, 2);

    pt_base = pd_entry & addr_mask;

    // Page Size bit set? (2M superpage)
    if ( page_size(pd_entry) )
    {
        maddr = offset_2M(pt_base, vaddr);
        if ( page_end )
            *page_end = roundup_2M(vaddr);
        return;
    }

    memory.read64(pt_base + pt_offset(vaddr),
                  pt_entry);

    // Page present?
    if ( ! present(pt_entry) )
        throw pagefault(vaddr, cr3, 1);

    page = pt_entry & addr_mask;
    maddr = offset_4K(page, vaddr);
    if ( page_end )
        *page_end = roundup_4K(vaddr);
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
