#pragma once

#include <string>
#include "elf.h"
#include "elf_string_tab.h"

class hash_tab;
class elf_symbol {
public:
    elf_symbol(elf32_sym *sym) {
        this->m_name_index      = (int)sym->st_name;
        this->m_value           = (addr_t)sym->st_value;
        this->m_size            = (size_t)sym->st_size;
        this->m_info            = (uint8_t)sym->st_info;
        this->m_other           = (uint8_t)sym->st_other;
        this->m_section_index   = (uint16_t)sym->st_shndx;
    }
    elf_symbol(elf64_sym *sym) {
        this->m_name_index      = (int)sym->st_name;
        this->m_info            = (uint8_t)sym->st_info;
        this->m_other           = (uint8_t)sym->st_other;
        this->m_section_index   = (uint16_t)sym->st_shndx;
        this->m_value           = (addr_t)sym->st_value;
        this->m_size            = (size_t)sym->st_size;
    }
    void set_sym_name(const char* sym_name) {
        this->m_name = sym_name;
    }
    const char* get_sym_name() {return m_name.c_str();}
//    const uint8_t get_type() {return m_type;}
    const addr_t get_value() {return m_value;}
    const size_t get_size() {return m_size;}
    const uint16_t get_section_index(){return m_section_index;}
    const int get_name_index() {return m_name_index;}
    const uint8_t get_info() {return m_info;}

protected:
    int         m_name_index;
    addr_t      m_value;
    size_t      m_size;
    uint8_t     m_info;
    uint8_t     m_other;
    uint16_t    m_section_index;
    std::string m_name;
};


class elf_symbol_tab {
public:
    elf_symbol_tab(Elf64_Sym* sym, elf_string_tab * strtab, hash_tab * hashtab) {
        this->m_sym_tab   = (addr_t)sym;
        this->m_elf_class = ELFCLASS64;
        this->m_str_tab   = strtab;
        this->m_hash_tab  = hashtab;
    }
    elf_symbol_tab(Elf32_Sym* sym, elf_string_tab * strtab, hash_tab * hashtab) {
        this->m_sym_tab   = (addr_t)sym;
        this->m_elf_class = ELFCLASS32;
        this->m_str_tab   = strtab;
        this->m_hash_tab  = hashtab;
    }
    ~elf_symbol_tab();

    elf_symbol* get_symbol(const int i) {
        elf_symbol* symbol = NULL;
        if (m_elf_class == ELFCLASS32) {
            Elf32_Sym* sym = &((Elf32_Sym*)m_sym_tab)[i];
            printf("sym: %d %p:\n", i, sym);
            symbol = new elf_symbol(sym);
        } else if (m_elf_class == ELFCLASS64) {
            Elf64_Sym* sym = &((Elf64_Sym*)m_sym_tab)[i];
            printf("sym: %d, %p:\n", i, sym);
            symbol = new elf_symbol(sym);
        }
        if (symbol && m_str_tab) {
            const char * name = m_str_tab->get_string(symbol->get_name_index());
            symbol->set_sym_name(name);
        }
        return symbol;
    }
protected:
    uint8_t         m_elf_class;
    addr_t          m_sym_tab;
    elf_string_tab* m_str_tab;
    hash_tab*       m_hash_tab;
};
