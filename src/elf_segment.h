#pragma once

#include "elf.h"

/*
typedef struct elf32_phdr {
  Elf32_Word p_type;
  Elf32_Off p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  Elf32_Word p_filesz;
  Elf32_Word p_memsz;
  Elf32_Word p_flags;
  Elf32_Word p_align;
} Elf32_Phdr;
typedef struct elf64_phdr {
  Elf64_Word p_type;
  Elf64_Word p_flags;
  Elf64_Off p_offset;
  Elf64_Addr p_vaddr;
  Elf64_Addr p_paddr;
  Elf64_Xword p_filesz;
  Elf64_Xword p_memsz;
  Elf64_Xword p_align;
} Elf64_Phdr;
*/

class elf_segment {

public:
    elf_segment(Elf32_Phdr *phdr) {
        this->m_type        = (uint32_t)phdr->p_type;
        this->m_flags       = (uint32_t)phdr->p_flags;
        this->m_offset      = (uint64_t)phdr->p_offset;
        this->m_virt_addr   = (uint64_t)phdr->p_vaddr;
        this->m_phy_addr    = (uint64_t)phdr->p_paddr;
        this->m_file_size   = (uint64_t)phdr->p_filesz;
        this->m_mem_size    = (uint64_t)phdr->p_memsz;
        this->m_align       = (uint64_t)phdr->p_align;
    }
    elf_segment(Elf64_Phdr *phdr) {
        this->m_type        = (uint32_t)phdr->p_type;
        this->m_flags       = (uint32_t)phdr->p_flags;
        this->m_offset      = (uint64_t)phdr->p_offset;
        this->m_virt_addr   = (uint64_t)phdr->p_vaddr;
        this->m_phy_addr    = (uint64_t)phdr->p_paddr;
        this->m_file_size   = (uint64_t)phdr->p_filesz;
        this->m_mem_size    = (uint64_t)phdr->p_memsz;
        this->m_align       = (uint64_t)phdr->p_align;
    }
    
    ~elf_segment();

    const uint32_t get_type() { return m_type; }
    const uint32_t get_flags() { return m_flags; }
    const uint64_t get_offset() { return m_offset; }
    const uint64_t get_virt_addr() { return m_virt_addr; }
    const uint64_t get_phy_addr() { return m_phy_addr; }
    const uint64_t get_file_size() { return m_file_size; }
    const uint64_t get_mem_size() { return m_mem_size; }
    const uint64_t get_alige() { return m_align; }

protected:

    elf_segment();
    uint32_t m_type;
    uint32_t m_flags;
    uint64_t m_offset;
    uint64_t m_virt_addr;
    uint64_t m_phy_addr;
    uint64_t m_file_size;
    uint64_t m_mem_size;
    uint64_t m_align;
};