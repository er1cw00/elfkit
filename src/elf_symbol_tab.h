#pragma once

#include <string>
#include "elf.h"
#include "elf_symbol.h"
#include "elf_string_tab.h"


class hash_tab;
class elf_symbol_tab {
public:
    elf_symbol_tab(Elf64_Sym* sym, elf_string_tab * strtab, hash_tab * hashtab) {
        log_trace("elf_symbol_tab ctor: %p\n", this);
        this->m_sym_tab   = (addr_t)sym;
        this->m_elf_class = ELFCLASS64;
        this->m_str_tab   = strtab;
        this->m_hash_tab  = hashtab;
    }
    elf_symbol_tab(Elf32_Sym* sym, elf_string_tab * strtab, hash_tab * hashtab) {
        log_trace("elf_symbol_tab ctor: %p\n", this);
        this->m_sym_tab   = (addr_t)sym;
        this->m_elf_class = ELFCLASS32;
        this->m_str_tab   = strtab;
        this->m_hash_tab  = hashtab;
    }
    ~elf_symbol_tab() {
        log_trace("elf_symbol_tab dtor: %p\n", this);
    }

    bool get_symbol(const int i, elf_symbol* symbol) {
        if (symbol) {
            if (m_elf_class == ELFCLASS32) {
                Elf32_Sym* sym = &((Elf32_Sym*)m_sym_tab)[i];
                elf_symbol_reset_with_sym32(symbol, sym);
            } else if (m_elf_class == ELFCLASS64) {
                Elf64_Sym* sym = &((Elf64_Sym*)m_sym_tab)[i];
                elf_symbol_reset_with_sym64(symbol, sym);
            } else {
                return false;
            }
            if (symbol && m_str_tab) {
                const char * name = m_str_tab->get_string(symbol->st_name);
                symbol->sym_name = (char*)name;
                return true;
            }
        }
        return false;
    }
protected:
    uint8_t         m_elf_class;
    addr_t          m_sym_tab;
    elf_string_tab* m_str_tab;
    hash_tab*       m_hash_tab;
};
