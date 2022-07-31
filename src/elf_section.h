#pragma once

#include "elf_common.h"

struct elf_section {
    int32_t  sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
};

typedef struct elf_section elf_section;

void elf_section_reset_with_shdr32(elf_section* section, Elf32_Shdr* shdr);
void elf_section_reset_with_shdr64(elf_section* section, Elf64_Shdr* shdr);