#pragma once

#include "elf.h"
#include "elf_common.h"
#include <assert.h>

struct elf_reloc {
    addr_t   r_offset;
    uint64_t r_info;
    int64_t  r_addend;
    uint8_t  r_elf_class;
};

typedef struct elf_reloc elf_reloc;

void elf_reloc_reset_with_rel32(elf_reloc * reloc, const Elf32_Rel * rel);
void elf_reloc_reset_with_rel64(elf_reloc * reloc, const Elf64_Rel * rel);
void elf_reloc_reset_with_rela32(elf_reloc * reloc, const Elf32_Rela * rela);
void elf_reloc_reset_with_rela64(elf_reloc * reloc, const Elf64_Rela * rela);

int elf_reloc_get_symbol_index(elf_reloc * reloc);
int elf_reloc_get_symbol_type(elf_reloc * reloc);
