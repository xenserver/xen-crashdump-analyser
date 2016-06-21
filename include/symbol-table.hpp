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

#ifndef __SYMBOL_TABLE_HPP__
#define __SYMBOL_TABLE_HPP__

/**
 * @file include/symbol-table.hpp
 * @author Andrew Cooper
 */

#include "util/symbol.hpp"
#include <map>
#include <list>
#include <utility>

#include <cstdio>

/**
 * Symbol table.
 * Symbols need to be indexed by name (to find specific data in memory),
 * and by address (to generate a stack trace).  Therefore, maintain two
 * mappings to the same symbol objects; one a mapping of virtual address
 * to symbol, and one a mapping of name to symbol.
 */
class SymbolTable
{
public:

    /**
     * Constructor.
     * Does very little.
     */
    SymbolTable();

    /**
     * Destructor.
     */
    ~SymbolTable();

    /**
     * Look up a Symbol by name.
     * @param name Name of a symbol
     * @returns pointer to a Symbol, or NULL if not found.
     */
    const Symbol * find(const char * name) const;

    /**
     * Parse a symbol file.
     * @param path Path to the symbol file.
     * @param offsets Whether to check for offset symbols.
     * @returns boolean indicating success.
     */
    bool parse(const char * path, bool offsets = false);

    /**
     * Print a 32bit symbol.
     *
     * @param stream Stream to print to.
     * @param addr Address of symbol.
     * @param brackets boolean indicating whether brackets should be printed.
     * @returns number of bytes written to stream.
     */
    int print_symbol32(FILE * stream, const vaddr_t & addr, bool brackets = false) const;

    /**
     * Print a 64bit symbol.
     *
     * @param stream Stream to print to.
     * @param addr Address of symbol.
     * @param brackets boolean indicating whether brackets should be printed.
     * @returns number of bytes written to stream.
     */
    int print_symbol64(FILE * stream, const vaddr_t & addr, bool brackets = false) const;

    /**
     * Print the text part of a symbol only.
     *
     * @param stream Stream to print to.
     * @param addr Address of symbol.
     * @returns number of bytes written to stream.
     */
    int print_text_symbol(FILE * stream, const vaddr_t & addr) const;

    /**
     * Is the address within one of the text regions.
     * @param addr Address to check.
     * @return boolean.
     */
    bool is_text_symbol(const vaddr_t & addr) const;

    /**
     * Add a new text region to the virtual address space.
     *
     * @param start Start address of the text region.
     * @param end End address of the text + 1.
     */
    void add_text_region(vaddr_t start, vaddr_t end);

    /**
     * Insert a new Symbol into the tables.
     * @param sym Symbol to insert.
     */
    void insert(Symbol * sym);

    /**
     * Sort the symbol table.
     * The symbol table must be sorted after inserting a symbol.
     */
    void sort();

protected:

    /// Whether this symbol table can print symbols.
    bool can_print;
    /// Whether this symbol table can decode hypercall pages.
    bool has_hypercall;

    /**
     * Private wrapper around std::strcmp.
     * Suitable as a comparison function for std::multimap
     * @param lhs Left hand side string.
     * @param rhs Right hand side string.
     * @returns boolean.
     */
    static bool strcmp(const char * lhs, const char * rhs);

    /**
     * Private helper for sorting list.  Sorts by symbol address.
     *
     * @param lhs Left hand side Symbol.
     * @param rhs Right hand side Symbol.
     * @returns boolean.
     */
    static bool addrcmp(const Symbol * lhs, const Symbol * rhs);

    /**
     * Private helper for searching lot.
     *
     * @param addr Address to compare.
     * @param sym Symbol to compare to.
     * @returns boolean.
     */
    static bool symcmp(const vaddr_t & addr, const Symbol * sym);

    /// value of 'hypercall_page' symbol.
    vaddr_t hypercall_page;

    /// Multimap of Symbol name -> Symbol
    std::multimap<const char *, Symbol *, bool(*)(const char*, const char*)> names;
    /// List of code symbols, for stack traces.
    std::list<Symbol*> symbols;

    /// List of text regions in the form of (start_addr, end_addr).
    std::list< std::pair<vaddr_t, vaddr_t> > text_regions;

    /// List iterator
    typedef std::list<Symbol*>::iterator list_iter;
    /// Constant list iterator
    typedef std::list<Symbol*>::const_iterator const_list_iter;

    /// Multimap pair
    typedef std::pair<const char *, Symbol*> name_pair;

    /// Multimap const iterator
    typedef std::multimap<const char *, Symbol *, bool(*)(const char*, const char*)>::const_iterator name_iter;

    /// Text region const iterator
    typedef std::list< std::pair<vaddr_t, vaddr_t> >::const_iterator text_region_iter;
};

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
