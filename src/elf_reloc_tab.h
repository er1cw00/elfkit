#pragma once

#include "elf_common.h"

struct elf_reloc {
    elf_reloc(Elf32_Rel * rel) {reset(rel);}
    elf_reloc(Elf64_Rel * rel) {reset(rel);}
    elf_reloc(Elf32_Rela * rela) {reset(rela);}
    elf_reloc(Elf64_Rela * rela) {reset(rela);}
    void reset(Elf32_Rel * rel) {
        assert(rel != NULL);
        this->r_offset      = (addr_t)rel->r_offset;
        this->r_info        = (uint64_t)rel->r_info;
        this->r_addend      = 0;
        this->r_elf_class   = ELFCLASS32;
    }
    void reset(Elf64_Rel * rel) {
        assert(rel != NULL);
        this->r_offset      = (addr_t)rel->r_offset;
        this->r_info        = (uint64_t)rel->r_info;
        this->r_addend      = 0;
        this->r_elf_class   = ELFCLASS64;
    }
    void reset(Elf32_Rela * rela) {
        assert(rela != NULL);
        this->r_offset      = (addr_t)rela->r_offset;
        this->r_info        = (uint64_t)rela->r_info;
        this->r_addend      = (int64_t)rela->r_addend;
        this->r_elf_class   = ELFCLASS32;
    }
    void reset(Elf64_Rela * rela) {
        assert(rela != NULL);
        this->r_offset      = (addr_t)rela->r_offset;
        this->r_info        = (uint64_t)rela->r_info;
        this->r_addend      = (int64_t)rela->r_addend;
        this->r_elf_class   = ELFCLASS64;
    }
    addr_t   r_offset;
    uint64_t r_info;
    int64_t  r_addend;
    uint8_t  r_elf_class;
};

typedef std::vector<elf_reloc*> elf_reloc_list_t;

class elf_reloc_tab {
public:
    elf_reloc_tab() {
        log_trace("elf_reloc_tab ctor: %p\n", this);        
        m_reloc_list.clear();
    }
    ~elf_reloc_tab() {
        log_trace("elf_reloc_tab dtor: %p\n", this);
        clear();
    }
    void clear() {
        for (elf_reloc_list_t::iterator itor = m_reloc_list.begin(); itor != m_reloc_list.end();) {
            elf_reloc* reloc = *itor;
            delete reloc;
            m_reloc_list.erase(itor);
        }
    }
    elf_reloc* get(const int i) {
        elf_reloc * reloc = NULL;
        if (i < m_reloc_list.size()) {
            reloc = m_reloc_list[i];
        }   
        return reloc;
    }
    void append(elf_reloc * reloc) {
        m_reloc_list.push_back(reloc);
    }
    size_t size() {
        return m_reloc_list.size();
    }
    bool empty() {
        return m_reloc_list.empty();
    }

protected:
    elf_reloc_list_t   m_reloc_list;
};
