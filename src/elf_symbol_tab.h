#pragma once

#include "elf.h"


class hash_tab;
class elf_symbol {
public:
    elf_symbol(elf32_sym *sym) {
        this->m_name_index      = (uint32_t)sym->st_name;
        this->m_value           = (addr_t)sym->st_value;
        this->m_size            = (size_t)sym->st_size;
        this->m_info            = (uint8_t)sym->st_info;
        this->m_other           = (uint8_t)sym->st_other;
        this->m_section_index   = (uint16_t)sym->st_shndx;
    }
    elf_symbol(elf64_sym *sym) {
        this->m_name_index      = (uint32_t)sym->st_name;
        this->m_info            = (uint8_t)sym->st_info;
        this->m_other           = (uint8_t)sym->st_other;
        this->m_section_index   = (uint16_t)sym->st_shndx;
        this->m_value           = (addr_t)sym->st_value;
        this->m_size            = (size_t)sym->st_size;
    }
    void set_name(const char* sym_name) {
        this->m_name = sym_name;
    }
    const char* get_sym_name() {return m_name.c_str();}
//    const uint8_t get_type() {return m_type;}
    const addr_t get_value() {return m_value;}
    const size_t get_size() {return m_size;}
    const uint16_t get_section_index(){return m_section_index;}
    const uint8_t get_info() {return m_info;}

protected:
    uint32_t    m_name_index;
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
        this->m_sym       = (addr_t)sym;
        this->m_elf_class = ELFCLASS64;
        this->m_str_tab   = strtab;
        this->m_hash_tab  = hashtab;
    }
    elf_symbol_tab(Elf32_Sym* sym, elf_string_tab * strtab, hash_tab * hashtab) {
        this->m_sym       = (addr_t)sym;
        this->m_elf_class = ELFCLASS32;
        this->m_str_tab   = strtab;
        this->m_hash_tab  = hashtab;
    }
    ~elf_symbol_tab();

    bool find_symboly_by_name(const char * name, elf_symbol & sym) {
        return false;
    }
    bool find_symbol_by_addr(const addr_t addr, elf_symbol & sym ) {
        return false;
    }

protected:
    uint8_t         m_elf_class;
    addr_t          m_sym;
    elf_string_tab* m_str_tab;
    hash_tab*       m_hash_tab;
};
