#pragma once

#include "elf_common.h"

class elf_reloc {
public:
    elf_reloc(Elf32_Rel * rel) {
        this->m_offset = (addr_t)rel->r_offset;
        this->m_info = (uint64_t)rel->r_info;
        this->m_addend = 0;
        this->m_elf_class = ELFCLASS32;
    }
    elf_reloc(Elf64_Rel * rel) {
        this->m_offset = (addr_t)rel->r_offset;
        this->m_info = (uint64_t)rel->r_info;
        this->m_addend = 0;
        this->m_elf_class = ELFCLASS64;
    }
    elf_reloc(Elf32_Rela * rela) {
        this->m_offset = (addr_t)rela->r_offset;
        this->m_info = (uint64_t)rela->r_info;
        this->m_addend = (int64_t)rela->r_addend;
        this->m_elf_class = ELFCLASS32;
    }
    elf_reloc(Elf64_Rela * rela) {
        this->m_offset = (addr_t)rela->r_offset;
        this->m_info = (uint64_t)rela->r_info;
        this->m_addend = (int64_t)rela->r_addend;
        this->m_elf_class = ELFCLASS64;
    }

protected:
    addr_t m_offset;
    uint64_t m_info;
    int64_t m_addend;
    uint8_t m_elf_class;
};
