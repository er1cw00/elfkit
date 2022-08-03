#pragma once

#include "elf_common.h"
#include "elf_reloc.h"


typedef std::vector<elf_reloc*> elf_reloc_list_t;

class elf_reloc_tab {
public:
    elf_reloc_tab(uint8_t elf_class, addr_t offset, size_t size, bool is_rela) {
        log_trace("elf_reloc_tab ctor: %p\n", this);  
        m_elf_class   = elf_class;
        m_offset      = offset;
        m_size        = size;  
        m_is_use_rela = is_rela;
    }
    ~elf_reloc_tab() {
        log_trace("elf_reloc_tab dtor: %p\n", this);
    }
    bool get_reloc(const int i, elf_reloc* reloc) {
        if (i >= m_size) {
            return false;
        }
        if (m_elf_class == ELFCLASS32) {
            if (m_is_use_rela) {
                Elf32_Rela* rela = (Elf32_Rela*)m_offset;
                elf_reloc_reset_with_rela32(reloc, &rela[i]);
            } else {
                Elf32_Rel* rel = (Elf32_Rel*)m_offset;
                elf_reloc_reset_with_rel32(reloc, &rel[i]);
            }
            return true;
        } else if (m_elf_class == ELFCLASS64){
            if (m_is_use_rela) {
                Elf64_Rela* rela = (Elf64_Rela*)m_offset;
                elf_reloc_reset_with_rela64(reloc, &rela[i]);
            } else {
                Elf64_Rel* rel = (Elf64_Rel*)m_offset;
                elf_reloc_reset_with_rel64(reloc, &rel[i]);
            }
            return true;
        }
    
        return false;
    }
    size_t size() {
        return m_size;
    }
protected:
    addr_t  m_offset;
    size_t  m_size;
    bool    m_is_use_rela;
    uint8_t m_elf_class;
};
