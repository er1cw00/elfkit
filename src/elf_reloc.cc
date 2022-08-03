
#include "elf.h"
#include "elf_reloc.h"

void elf_reloc_reset_with_rel32(elf_reloc * reloc, const Elf32_Rel * rel) {
    assert(rel != NULL);
    reloc->r_offset      = (addr_t)rel->r_offset;
    reloc->r_info        = (uint64_t)rel->r_info;
    reloc->r_addend      = 0;
    reloc->r_elf_class   = ELFCLASS32;
}
void elf_reloc_reset_with_rel64(elf_reloc * reloc, const Elf64_Rel * rel) {
    assert(rel != NULL);
    reloc->r_offset      = (addr_t)rel->r_offset;
    reloc->r_info        = (uint64_t)rel->r_info;
    reloc->r_addend      = 0;
    reloc->r_elf_class   = ELFCLASS64;
}
void elf_reloc_reset_with_rela32(elf_reloc * reloc, const Elf32_Rela * rela) {
    assert(rela != NULL);
    reloc->r_offset      = (addr_t)rela->r_offset;
    reloc->r_info        = (uint64_t)rela->r_info;
    reloc->r_addend      = (int64_t)rela->r_addend;
    reloc->r_elf_class   = ELFCLASS32;
}
void elf_reloc_reset_with_rela64(elf_reloc * reloc, const Elf64_Rela * rela) {
    assert(rela != NULL);
    reloc->r_offset      = (addr_t)rela->r_offset;
    reloc->r_info        = (uint64_t)rela->r_info;
    reloc->r_addend      = (int64_t)rela->r_addend;
    reloc->r_elf_class   = ELFCLASS64;
}