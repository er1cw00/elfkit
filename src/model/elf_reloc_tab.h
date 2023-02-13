#pragma once
#include <vector>

#include <common/elf.h>
#include <common/elf_log.h>

typedef std::vector<elf_reloc> elf_reloc_list_t;

class elf_reloc_tab {
public:
    elf_reloc_tab(uint8_t elf_class, addr_t offset, size_t size, bool is_rela);
    ~elf_reloc_tab();
    bool is_use_rela();
    bool get_reloc(const int i, elf_reloc* reloc);    
    elf_reloc_list_t & get_list();
    size_t size();
    
protected:
    void _reset_reloc(const int i, elf_reloc * reloc);

protected:
    addr_t  m_offset;
    size_t  m_size;
    uint8_t m_elf_class;
    bool    m_is_use_rela;
    elf_reloc_list_t m_reloc_list;
};
