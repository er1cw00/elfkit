#pragma once

#include "elf.h"


struct elf_segment {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;

    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
};

typedef struct elf_segment elf_segment;
void elf_segment_reset_with_phdr32(elf_segment *segment, Elf32_Phdr *phdr);
void elf_segment_reset_with_phdr64(elf_segment *segment, Elf64_Phdr *phdr);