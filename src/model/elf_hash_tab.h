#pragma once

#include <common/elf_log.h>
#include <common/elf_common.h>

class elf_symbol;
class elf_symbol_tab;
class elf_hash_tab {
public:
    elf_hash_tab() {
        log_trace("elf_hash_tab ctor: %p\n", this);
    }
    virtual ~elf_hash_tab() { 
        log_trace("elf_hash_tab dtor: %p\n", this);
    }

    virtual uint32_t get_hash_code(const char * name) = 0 ;
    virtual size_t get_symbol_nums() = 0; 
    virtual bool find_symbol_by_name(elf_symbol_tab* sym_tab, const char* name, elf_symbol* symbol) = 0;
    virtual bool find_symbol_by_addr(elf_symbol_tab* sym_tab, const addr_t addr, elf_symbol* symbol) = 0;
    virtual void dump_hash_table() = 0;

protected:
    bool _symbol_matches_soaddr(elf_symbol* sym, addr_t soaddr);
};
