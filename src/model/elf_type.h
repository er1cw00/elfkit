#pragma once

#include <common/elf.h>
#include <common/elf_common.h>

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

struct elf_dynamic {
    int64_t d_tag;
    uint64_t d_val;
};

typedef struct elf_dynamic elf_dynamic;
void elf_dynamic_reset_with_dyn32(elf_dynamic *dynamic, Elf32_Dyn* dyn);
void elf_dynamic_reset_with_dyn64(elf_dynamic *dynamic, Elf64_Dyn* dyn);


struct elf_symbol {

    int         st_name;
    addr_t      st_value;
    size_t      st_size;
    uint8_t     st_info;
    uint8_t     st_other;
    uint16_t    st_shndx;
    char*       sym_name;
};

typedef struct elf_symbol elf_symbol;

void elf_symbol_reset_with_sym32(elf_symbol *symbol, Elf32_Sym *sym);
void elf_symbol_reset_with_sym64(elf_symbol *symbol, Elf64_Sym *sym); 

struct elf_reloc {
    addr_t   r_offset;
    uint64_t r_info;
    int64_t  r_addend;
    uint8_t  r_elf_class;
    bool     r_use_rela;
};

typedef struct elf_reloc elf_reloc;

void elf_reloc_reset_with_rel32(elf_reloc * reloc, const Elf32_Rel * rel);
void elf_reloc_reset_with_rel64(elf_reloc * reloc, const Elf64_Rel * rel);
void elf_reloc_reset_with_rela32(elf_reloc * reloc, const Elf32_Rela * rela);
void elf_reloc_reset_with_rela64(elf_reloc * reloc, const Elf64_Rela * rela);

int elf_reloc_get_symbol_index(elf_reloc * reloc);
int elf_reloc_get_symbol_type(elf_reloc * reloc);
