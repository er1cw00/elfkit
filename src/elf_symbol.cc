
#include "elf.h"
#include "elf_symbol.h"

void elf_symbol_reset_with_sym32(elf_symbol* symbol, Elf32_Sym *sym) {
    symbol->st_name      = (int)sym->st_name;
    symbol->st_value     = (addr_t)sym->st_value;
    symbol->st_size      = (size_t)sym->st_size;
    symbol->st_info      = (uint8_t)sym->st_info;
    symbol->st_other     = (uint8_t)sym->st_other;
    symbol->st_shndx     = (uint16_t)sym->st_shndx;
    symbol->sym_name     = NULL;
}

void elf_symbol_reset_with_sym64(elf_symbol* symbol, Elf64_Sym *sym) {
    symbol->st_name      = (int)sym->st_name;
    symbol->st_info      = (uint8_t)sym->st_info;
    symbol->st_other     = (uint8_t)sym->st_other;
    symbol->st_shndx     = (uint16_t)sym->st_shndx;
    symbol->st_value     = (addr_t)sym->st_value;
    symbol->st_size      = (size_t)sym->st_size;
    symbol->sym_name     = NULL;
}