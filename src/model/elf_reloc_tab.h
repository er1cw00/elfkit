#pragma once

#include <common/elf.h>
#include <common/elf_log.h>

typedef std::vector<elf_reloc> elf_reloc_list_t;

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
    bool get_reloc(const int i, elf_reloc* reloc);    
    size_t get_list(elf_reloc_list_t & list);
    size_t size();
protected:
    void _reset_reloc(const int i, elf_reloc * reloc);

protected:
    addr_t  m_offset;
    size_t  m_size;
    uint8_t m_elf_class;
    bool    m_is_use_rela;
};
