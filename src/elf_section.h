#pragma once

#include "elf.h"

class elf_section {

public:
    elf_section(Elf32_Shdr* shdr) {
        assert(shdr != NULL);
        this->m_name_index  = (int32_t)shdr->sh_name;
        this->m_type        = (uint32_t)shdr->sh_type;
        this->m_flags       = (uint64_t)shdr->sh_flags;
        this->m_address     = (uint64_t)shdr->sh_addr;
        this->m_offset      = (uint64_t)shdr->sh_offset;
        this->m_size        = (uint64_t)shdr->sh_size;
        this->m_link        = (uint32_t)shdr->sh_link;
        this->m_info        = (uint32_t)shdr->sh_info;
        this->m_addr_align  = (uint64_t)shdr->sh_addralign;
        this->m_entry_size  = (uint64_t)shdr->sh_entsize;

    }
    elf_section(Elf64_Shdr* shdr) {
        assert(shdr != NULL);
        this->m_name_index  = (int32_t)shdr->sh_name;
        this->m_type        = (uint32_t)shdr->sh_type;
        this->m_flags       = (uint64_t)shdr->sh_flags;
        this->m_address     = (uint64_t)shdr->sh_addr;
        this->m_offset      = (uint64_t)shdr->sh_offset;
        this->m_size        = (uint64_t)shdr->sh_size;
        this->m_link        = (uint32_t)shdr->sh_link;
        this->m_info        = (uint32_t)shdr->sh_info;
        this->m_addr_align  = (uint64_t)shdr->sh_addralign;
        this->m_entry_size  = (uint64_t)shdr->sh_entsize;
    }

    ~elf_section();

    const int32_t  get_name_index() { return m_name_index; }
    const uint32_t get_type() { return m_type; }
    const uint64_t get_flags() { return m_flags; }
    const uint64_t get_address() { return m_address; }
    const uint64_t get_offset() { return m_offset; }
    const uint64_t get_size() { return m_size;}
    const uint32_t get_link() { return m_link; }
    const uint32_t get_info() { return m_info; }
    const uint64_t get_addr_align() { return m_addr_align; }
    const uint64_t get_entry_size() { return m_entry_size; }

protected:

    elf_section();
    int32_t  m_name_index;
    uint32_t m_type;
    uint64_t m_flags;
    uint64_t m_address;
    uint64_t m_offset;
    uint64_t m_size;
    uint32_t m_link;
    uint32_t m_info;
    uint64_t m_addr_align;
    uint64_t m_entry_size;
};

