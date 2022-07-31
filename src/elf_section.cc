
#include "elf.h"
#include "elf_section.h"
#include <assert.h>

void elf_section_reset_with_shdr32(elf_section* section, Elf32_Shdr* shdr) {
    assert(shdr != NULL);
    section->sh_name        = (int32_t)shdr->sh_name;
    section->sh_type        = (uint32_t)shdr->sh_type;
    section->sh_flags       = (uint64_t)shdr->sh_flags;
    section->sh_addr        = (uint64_t)shdr->sh_addr;
    section->sh_offset      = (uint64_t)shdr->sh_offset;
    section->sh_size        = (uint64_t)shdr->sh_size;
    section->sh_link        = (uint32_t)shdr->sh_link;
    section->sh_info        = (uint32_t)shdr->sh_info;
    section->sh_addralign   = (uint64_t)shdr->sh_addralign;
    section->sh_entsize     = (uint64_t)shdr->sh_entsize;
}

void elf_section_reset_with_shdr64(elf_section* section, Elf64_Shdr* shdr) {
    assert(shdr != NULL);
    section->sh_name        = (int32_t)shdr->sh_name;
    section->sh_type        = (uint32_t)shdr->sh_type;
    section->sh_flags       = (uint64_t)shdr->sh_flags;
    section->sh_addr        = (uint64_t)shdr->sh_addr;
    section->sh_offset      = (uint64_t)shdr->sh_offset;
    section->sh_size        = (uint64_t)shdr->sh_size;
    section->sh_link        = (uint32_t)shdr->sh_link;
    section->sh_info        = (uint32_t)shdr->sh_info;
    section->sh_addralign   = (uint64_t)shdr->sh_addralign;
    section->sh_entsize     = (uint64_t)shdr->sh_entsize;
}