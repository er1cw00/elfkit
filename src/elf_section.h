#pragma once

#include "elf.h"

struct elf_section {

    elf_section(Elf32_Shdr* shdr) {
        reset(shdr);
    }
    elf_section(Elf64_Shdr* shdr) {
        reset(shdr);
    }
    ~elf_section();
    void reset(Elf32_Shdr* shdr) {
        assert(shdr != NULL);
        this->sh_name        = (int32_t)shdr->sh_name;
        this->sh_type        = (uint32_t)shdr->sh_type;
        this->sh_flags       = (uint64_t)shdr->sh_flags;
        this->sh_addr        = (uint64_t)shdr->sh_addr;
        this->sh_offset      = (uint64_t)shdr->sh_offset;
        this->sh_size        = (uint64_t)shdr->sh_size;
        this->sh_link        = (uint32_t)shdr->sh_link;
        this->sh_info        = (uint32_t)shdr->sh_info;
        this->sh_addralign   = (uint64_t)shdr->sh_addralign;
        this->sh_entsize     = (uint64_t)shdr->sh_entsize;
    }
    void reset(Elf64_Shdr* shdr) {
        assert(shdr != NULL);
        this->sh_name        = (int32_t)shdr->sh_name;
        this->sh_type        = (uint32_t)shdr->sh_type;
        this->sh_flags       = (uint64_t)shdr->sh_flags;
        this->sh_addr        = (uint64_t)shdr->sh_addr;
        this->sh_offset      = (uint64_t)shdr->sh_offset;
        this->sh_size        = (uint64_t)shdr->sh_size;
        this->sh_link        = (uint32_t)shdr->sh_link;
        this->sh_info        = (uint32_t)shdr->sh_info;
        this->sh_addralign   = (uint64_t)shdr->sh_addralign;
        this->sh_entsize     = (uint64_t)shdr->sh_entsize;
    }
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

