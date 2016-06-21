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

/**
 * @file src/symbol-table.cpp
 * @author Andrew Cooper
 */
#include "symbol-table.hpp"
#include "util/log.hpp"
#include "util/macros.hpp"
#include "util/stdio-wrapper.hpp"

#include <cstring>
#include <cstdio>
#include <algorithm>

#include "util/xensym-common.hpp"
#include "abstract/xensyms.hpp"
#include "arch/x86_64/xensyms.hpp"

/// Hypercall number -> name
static const char *hypercall_names[] = {
	/*0 =*/ "__HYPERVISOR_set_trap_table",
	/*1 =*/ "__HYPERVISOR_mmu_update",
	/*2 =*/ "__HYPERVISOR_set_gdt",
	/*3 =*/ "__HYPERVISOR_stack_switch",
	/*4 =*/ "__HYPERVISOR_set_callbacks",
	/*5 =*/ "__HYPERVISOR_fpu_taskswitch",
        NULL,
	/*7 =*/ "__HYPERVISOR_platform_op",
	/*8 =*/ "__HYPERVISOR_set_debugreg",
	/*9 =*/ "__HYPERVISOR_get_debugreg",
	/*10 =*/ "__HYPERVISOR_update_descriptor",
        NULL,
	/*12 =*/ "__HYPERVISOR_memory_op",
	/*13 =*/ "__HYPERVISOR_multicall",
	/*14 =*/ "__HYPERVISOR_update_va_mapping",
	/*15 =*/ "__HYPERVISOR_set_timer_op",
        NULL,
	/*17 =*/ "__HYPERVISOR_xen_version",
	/*18 =*/ "__HYPERVISOR_console_io",
        NULL,
	/*20 =*/ "__HYPERVISOR_grant_table_op",
	/*21 =*/ "__HYPERVISOR_vm_assist",
	/*22 =*/ "__HYPERVISOR_update_va_mapping_otherdomain",
        NULL,
	/*24 =*/ "__HYPERVISOR_vcpu_op",
        NULL,
	/*26 =*/ "__HYPERVISOR_mmuext_op",
	/*27 =*/ "__HYPERVISOR_xsm_op",
	/*28 =*/ "__HYPERVISOR_nmi_op",
	/*29 =*/ "__HYPERVISOR_sched_op",
	/*30 =*/ "__HYPERVISOR_callback_op",
	/*31 =*/ "__HYPERVISOR_xenoprof_op",
	/*32 =*/ "__HYPERVISOR_event_channel_op",
	/*33 =*/ "__HYPERVISOR_physdev_op",
	/*34 =*/ "__HYPERVISOR_hvm_op",
	/*35 =*/ "__HYPERVISOR_sysctl",
	/*36 =*/ "__HYPERVISOR_domctl",
	/*37 =*/ "__HYPERVISOR_kexec_op",
	/*38 =*/ "__HYPERVISOR_tmem_op",
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
	/*48 =*/ "__HYPERVISOR_arch_0",
	/*49 =*/ "__HYPERVISOR_arch_1",
	/*50 =*/ "__HYPERVISOR_arch_2",
	/*51 =*/ "__HYPERVISOR_arch_3",
	/*52 =*/ "__HYPERVISOR_arch_4",
	/*53 =*/ "__HYPERVISOR_arch_5",
	/*54 =*/ "__HYPERVISOR_arch_6",
	/*55 =*/ "__HYPERVISOR_arch_7",
};

/**
 * Find hypercall name from a number
 * @param nr Hypercall number.
 * @return hypercall name, or suitable error information.
 */
static const char * hypercall_name(const unsigned int nr)
{
    if ( nr < sizeof hypercall_names / sizeof hypercall_names[0] )
        return hypercall_names[nr] ? hypercall_names[nr] : "unknown";
    return "out of range";
}

SymbolTable::SymbolTable():
    can_print(false), has_hypercall(false),
    hypercall_page(0), names(&SymbolTable::strcmp), symbols(), text_regions()
{}

SymbolTable::~SymbolTable()
{
    for ( name_iter itt = this->names.begin();
          itt != this->names.end(); ++itt)
        delete itt->second;
    this->names.clear();
    this->symbols.clear();
}

void SymbolTable::insert(Symbol * sym)
{
    if ( sym->type == 'T' ||
         sym->type == 't' ||
         sym->type == 'W' ||
         sym->type == 'w' )
        this->symbols.push_back(sym);

    this->names.insert(name_pair(sym->name, sym));
}

void SymbolTable::sort()
{
    this->symbols.sort(&SymbolTable::addrcmp);
}

const Symbol * SymbolTable::find(const char * name) const
{
    // If we are asked for a symbol by name and more than one of said symbol
    // is present, give up.
    if ( this->names.count(name) > 1 )
    {
        LOG_INFO("Found more than one symbol with name '%s'\n", name);
        return NULL;
    }
    name_iter f = this->names.find(name);
    return f == this->names.end() ? NULL : f->second;
}

bool SymbolTable::parse(const char * file, bool offsets)
{
    FILE * fd = NULL;
    vaddr_t addr;
    char type;
    char name[128];
    int nr_read;
    vaddr_t text_start = 0, text_end = 0, init_start = 0, init_end = 0;

    if ( NULL == (fd = fopen(file, "r")) )
        return false;

    while ( true )
    {
        nr_read = fscanf(fd, "%" SCNx64 " %c %127s", &addr, &type, name);

        if ( nr_read != 3 )
        {
            if ( feof(fd) )
                break;
            SAFE_FCLOSE(fd);
            return false;
        }

        if ( name[0] == '+' )
        {
            if ( offsets )
            {
                insert_xensym(Abstract::xensyms::xensyms, &name[1], addr);
                insert_xensym(x86_64::xensyms::xensyms, &name[1], addr);
            }
        }
        else
        {
            if ( offsets )
            {
                insert_xensym(Abstract::xensyms::xensyms, name, addr);
                insert_xensym(x86_64::xensyms::xensyms, name, addr);
            }

            if ( ! std::strcmp(name, "_stext") )
                text_start = addr;
            else if ( ! std::strcmp(name, "_etext") )
                text_end = addr;
            else if ( ! std::strcmp(name, "_sinittext") )
                init_start = addr;
            else if ( ! std::strcmp(name, "_einittext") )
                init_end = addr;
            else if ( ! std::strcmp(name, "hypercall_page") )
                this->hypercall_page = addr;

            this->insert( new Symbol(addr, type, name) );
        }
    }

    SAFE_FCLOSE(fd);

    sort();

    if ( text_start == 0 ||
         text_end == 0 ||
         init_start == 0 ||
         init_end == 0 )
    {
        LOG_INFO("Failed to obtain text section limits\n");
        this->can_print = false;
    }
    else
    {
        add_text_region(text_start, text_end);
        add_text_region(init_start, init_end);

        LOG_DEBUG("  text section limits: 0x%016"PRIx64"->0x%016"PRIx64"\n",
                  text_start, text_end);
        LOG_DEBUG("  init section limits: 0x%016"PRIx64"->0x%016"PRIx64"\n",
                  init_start, init_end);
        this->can_print = true;
    }

    if ( this->hypercall_page == 0 )
        this->has_hypercall = false;
    else
    {
        this->has_hypercall = true;
        LOG_DEBUG("  hypercall page:      0x%016"PRIx64"->0x%016"PRIx64"\n",
                  this->hypercall_page, this->hypercall_page+4096);
    }

    return true;
}

int SymbolTable::print_symbol64(FILE * o, const vaddr_t & addr, bool brackets) const
{
    int len = 0;

    if ( ! this->can_print )
        return 0;

    if ( ! this->is_text_symbol(addr) )
        return 0;

    SymbolTable::const_list_iter after
        = std::upper_bound(this->symbols.begin(), this->symbols.end(), addr, &SymbolTable::symcmp);

    if ( after == this->symbols.begin() ||
         after == this->symbols.end() )
        return 0;

    SymbolTable::const_list_iter before = after;
    before--;

    if ( (*before)->address <= addr && (*after)->address > addr )
    {
        len += FPUTS("\t ", o);
        if ( brackets )
            len += FPRINTF(o, "[%016"PRIx64"]", addr);
        else
            len += FPRINTF(o, " %016"PRIx64" ", addr);

        len += FPRINTF(o, " %s+%#"PRIx64"/%#"PRIx64,
                       (*before)->name,
                       addr - (*before)->address,
                       (*after)->address - (*before)->address );

        if ( ! std::strcmp((*before)->name, "hypercall_page") )
        {
            unsigned int nr = (unsigned int)((addr - (*before)->address)/32);
            len += FPRINTF(o, " (%d, %s)", nr, hypercall_name(nr));
        }

        len += FPUTS("\n", o);
    }
    else
        LOG_WARN("Strange resulting iterators printing symbol 0x%016"PRIx64"\n", addr);

    return len;
}
int SymbolTable::print_symbol32(FILE * o, const vaddr_t & addr, bool brackets) const
{
    int len = 0;

    if ( ! this->can_print )
        return 0;

    if ( ! this->is_text_symbol(addr) )
        return 0;

    SymbolTable::const_list_iter after
        = std::upper_bound(this->symbols.begin(), this->symbols.end(), addr, &SymbolTable::symcmp);

    if ( after == this->symbols.begin() ||
         after == this->symbols.end() )
        return 0;

    SymbolTable::const_list_iter before = after;
    before--;

    if ( (*before)->address <= addr && (*after)->address > addr )
    {
        len += FPUTS("\t ", o);
        if ( brackets )
            len += FPRINTF(o, "[%08"PRIx64"]", addr);
        else
            len += FPRINTF(o, " %08"PRIx64" ", addr);

        len += FPRINTF(o, " %s+%#"PRIx64"/%#"PRIx64,
                       (*before)->name,
                       addr - (*before)->address,
                       (*after)->address - (*before)->address );

        if ( ! std::strcmp((*before)->name, "hypercall_page") )
        {
            unsigned int nr = (unsigned int)((addr - (*before)->address)/32);
            len += FPRINTF(o, " (%d, %s)", nr, hypercall_name(nr));
        }

        len += FPUTS("\n", o);
    }
    else
        LOG_WARN("Strange resulting iterators printing symbol 0x%016"PRIx64"\n", addr);

    return len;
}

int SymbolTable::print_text_symbol(FILE * o, const vaddr_t & addr) const
{
    int len = 0;

    if ( ! this->can_print )
        return 0;

    if ( ! this->is_text_symbol(addr) )
        return 0;

    SymbolTable::const_list_iter after
        = std::upper_bound(this->symbols.begin(), this->symbols.end(), addr, &SymbolTable::symcmp);

    if ( after == this->symbols.begin() ||
         after == this->symbols.end() )
        return 0;

    SymbolTable::const_list_iter before = after;
    before--;

    if ( (*before)->address <= addr && (*after)->address > addr )
    {
        len += FPRINTF(o, "%s+%#"PRIx64"/%#"PRIx64,
                       (*before)->name,
                       addr - (*before)->address,
                       (*after)->address - (*before)->address );
    }
    else
        LOG_WARN("Strange resulting iterators printing symbol 0x%016"PRIx64"\n", addr);

    return len;
}

bool SymbolTable::is_text_symbol(const vaddr_t & addr) const
{
    if ( ! this->can_print )
        return false;

    if ( this->has_hypercall &&
         addr >= this->hypercall_page &&
         addr < this->hypercall_page + 4096ULL )
        return true;

    for ( text_region_iter itt = text_regions.begin();
          itt != text_regions.end(); ++itt )
    {
        if ( addr >= itt->first && addr < itt->second )
            return true;
    }

    return false;
}

bool SymbolTable::strcmp(const char * lhs, const char * rhs)
{
    return std::strcmp(lhs, rhs) < 0;
}

bool SymbolTable::addrcmp(const Symbol * lhs, const Symbol * rhs)
{
    return lhs->address < rhs->address;
}

bool SymbolTable::symcmp(const vaddr_t & addr, const Symbol * sym)
{
    return addr < sym->address;
}

void SymbolTable::add_text_region(vaddr_t start, vaddr_t end)
{
    text_regions.push_back(std::pair<vaddr_t, vaddr_t>(start, end));
}

/*
 * Local variables:
 * mode: C++
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
