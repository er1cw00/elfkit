#pragma once

#include <common/elf.h>
#include <common/elf_log.h>

struct elf_symbol;
class elf_hash_tab;
class elf_string_tab;
class elf_symbol_tab {
public:
    elf_symbol_tab(Elf64_Sym* sym, elf_string_tab * strtab, elf_hash_tab * hashtab) {
        log_trace("elf_symbol_tab ctor: %p\n", this);
        this->m_sym_tab   = (addr_t)sym;
        this->m_elf_class = ELFCLASS64;
        this->m_str_tab   = strtab;
        this->m_hash_tab  = hashtab;
    }
    elf_symbol_tab(Elf32_Sym* sym, elf_string_tab * strtab, elf_hash_tab * hashtab) {
        log_trace("elf_symbol_tab ctor: %p\n", this);
        this->m_sym_tab   = (addr_t)sym;
        this->m_elf_class = ELFCLASS32;
        this->m_str_tab   = strtab;
        this->m_hash_tab  = hashtab;
    }
    ~elf_symbol_tab() {
        log_trace("elf_symbol_tab dtor: %p\n", this);
    }
    bool get_symbol(const int i, elf_symbol* symbol);
protected:
    uint8_t         m_elf_class;
    addr_t          m_sym_tab;
    elf_string_tab* m_str_tab;
    elf_hash_tab*   m_hash_tab;
};
