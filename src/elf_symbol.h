#pragma once

#include "elf.h"
#include "elf_common.h"

struct elf_symbol {

    int         st_name;
    addr_t      st_value;
    size_t      st_size;
    uint8_t     st_info;
    uint8_t     st_other;
    uint16_t    st_shndx;
    char*        sym_name;
};

void elf_symbol_reset_with_sym32(elf_symbol *symbol, Elf32_Sym *sym);
void elf_symbol_reset_with_sym64(elf_symbol *symbol, Elf64_Sym *sym); 