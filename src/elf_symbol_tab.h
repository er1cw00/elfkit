#pragma once

#include <string>
#include "elf.h"
#include "elf_string_tab.h"

class hash_tab;
struct elf_symbol {
    elf_symbol() {} 
    elf_symbol(elf32_sym *sym) {reset(sym);} 
    elf_symbol(elf64_sym *sym) {reset(sym);}
    void reset(elf32_sym *sym) {
        this->st_name      = (int)sym->st_name;
        this->st_value     = (addr_t)sym->st_value;
        this->st_size      = (size_t)sym->st_size;
        this->st_info      = (uint8_t)sym->st_info;
        this->st_other     = (uint8_t)sym->st_other;
        this->st_shndx     = (uint16_t)sym->st_shndx;
    }
    void reset(elf64_sym *sym) {
        this->st_name      = (int)sym->st_name;
        this->st_info      = (uint8_t)sym->st_info;
        this->st_other     = (uint8_t)sym->st_other;
        this->st_shndx     = (uint16_t)sym->st_shndx;
        this->st_value     = (addr_t)sym->st_value;
        this->st_size      = (size_t)sym->st_size;
    }
    void set_sym_name(const char* sym_name) {
        this->sym_name = sym_name;
    }
    const char* get_sym_name() {
        return sym_name.c_str();
    }

    const int get_name_index() {return st_name;}
    const addr_t get_value() {return st_value;}
    const size_t get_size() {return st_size;}
    const uint8_t get_info() {return st_info;}
    const uint8_t get_other() {return st_other;}
    const uint16_t get_shndx() {return st_shndx;}

    int         st_name;
    addr_t      st_value;
    size_t      st_size;
    uint8_t     st_info;
    uint8_t     st_other;
    uint16_t    st_shndx;
    std::string sym_name;
};


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
                symbol->reset(sym);
            } else if (m_elf_class == ELFCLASS64) {
                Elf64_Sym* sym = &((Elf64_Sym*)m_sym_tab)[i];
                symbol->reset(sym);
            } else {
                return false;
            }
            if (symbol && m_str_tab) {
                const char * name = m_str_tab->get_string(symbol->st_name);
                symbol->set_sym_name(name);
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
