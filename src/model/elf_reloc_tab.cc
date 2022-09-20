#include <assert.h>
#include <vector>

#include <common/elf.h>
#include <common/elf_log.h>
#include <model/elf_type.h>
#include <model/elf_reloc_tab.h>

elf_reloc_tab::elf_reloc_tab(uint8_t elf_class, addr_t offset, size_t size, bool is_rela) {
    log_trace("elf_reloc_tab ctor: %p, %d\n", this, elf_class);  
    m_elf_class   = elf_class;
    m_offset      = offset;
    m_size        = size;  
    m_is_use_rela = is_rela;
    m_reloc_list.clear();
    m_reloc_list.resize(m_size);
    for (int i = 0; i < m_size; i++) {
        elf_reloc reloc;
        _reset_reloc(i, &reloc);
        m_reloc_list[i] = reloc;
    }
}

elf_reloc_tab::~elf_reloc_tab() {
    log_trace("elf_reloc_tab dtor: %p\n", this);
}

bool elf_reloc_tab::get_reloc(const int i, elf_reloc* reloc) {
    if (i >= m_size) {
        return false;
    }
    _reset_reloc(i, reloc);
    return false;
}

size_t elf_reloc_tab::size() {
    return m_size;
}

elf_reloc_list_t& elf_reloc_tab::get_list() {
    return m_reloc_list;
}

void elf_reloc_tab::_reset_reloc(const int i, elf_reloc * reloc) {
    assert(m_elf_class == ELFCLASS32 || m_elf_class == ELFCLASS64);
    if (m_elf_class == ELFCLASS32) {
        if (m_is_use_rela) {
            Elf32_Rela* rela = (Elf32_Rela*)m_offset;
            elf_reloc_reset_with_rela32(reloc, &rela[i]);
        } else {
            Elf32_Rel* rel = (Elf32_Rel*)m_offset;
            elf_reloc_reset_with_rel32(reloc, &rel[i]);
        }
    } else if (m_elf_class == ELFCLASS64){
        if (m_is_use_rela) {
            Elf64_Rela* rela = (Elf64_Rela*)m_offset;
            elf_reloc_reset_with_rela64(reloc, &rela[i]);
        } else {
            Elf64_Rel* rel = (Elf64_Rel*)m_offset;
            elf_reloc_reset_with_rel64(reloc, &rel[i]);
        }
    }
}