#pragma once

#include <common/elf.h>
#include <common/elf_log.h>

struct elf_symbol;
class elf_hash_tab;
class elf_string_tab;
class elf_symbol_tab {
public:
    elf_symbol_tab(Elf64_Sym* symtab, const size_t symtab_size, elf_string_tab * strtab) {
        log_trace("elf_symbol_tab ctor: %p\n", this);
        this->m_symtab       = (addr_t)symtab;
        this->m_symtab_size  = symtab_size;
        this->m_strtab       = strtab;
        this->m_elf_class    = ELFCLASS64;
    }
    elf_symbol_tab(Elf32_Sym* symtab, const size_t symtab_size, elf_string_tab * strtab) {
        log_trace("elf_symbol_tab ctor: %p\n", this);
        this->m_symtab       = (addr_t)symtab;
        this->m_symtab_size  = symtab_size;
        this->m_strtab       = strtab;
        this->m_elf_class    = ELFCLASS32;   
    }
    ~elf_symbol_tab() {
        log_trace("elf_symbol_tab dtor: %p\n", this);
    }
    bool get_symbol(const int i, elf_symbol* symbol);
protected:
    uint8_t         m_elf_class;
    addr_t          m_symtab;
    size_t          m_symtab_size;
    elf_string_tab* m_strtab;
};
