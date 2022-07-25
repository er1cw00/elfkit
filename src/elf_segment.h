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

struct elf_segment {
    elf_segment(Elf32_Phdr* phdr) {
        reset(phdr);
    }
    elf_segment(Elf64_Phdr* phdr) {
        reset(phdr);
    }
    ~elf_segment();
    void reset(Elf32_Phdr *phdr) {
        this->p_type        = (uint32_t)phdr->p_type;
        this->p_flags       = (uint32_t)phdr->p_flags;
        this->p_offset      = (uint64_t)phdr->p_offset;
        this->p_vaddr       = (uint64_t)phdr->p_vaddr;
        this->p_paddr       = (uint64_t)phdr->p_paddr;
        this->p_filesz      = (uint64_t)phdr->p_filesz;
        this->p_memsz       = (uint64_t)phdr->p_memsz;
        this->p_align       = (uint64_t)phdr->p_align;
    }
    void reset(Elf64_Phdr *phdr) {
        this->p_type        = (uint32_t)phdr->p_type;
        this->p_flags       = (uint32_t)phdr->p_flags;
        this->p_offset      = (uint64_t)phdr->p_offset;
        this->p_vaddr       = (uint64_t)phdr->p_vaddr;
        this->p_paddr       = (uint64_t)phdr->p_paddr;
        this->p_filesz      = (uint64_t)phdr->p_filesz;
        this->p_memsz       = (uint64_t)phdr->p_memsz;
        this->p_align       = (uint64_t)phdr->p_align;
    }
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
};