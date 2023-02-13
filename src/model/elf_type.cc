#include <assert.h>

#include <model/elf_type.h>

void elf_section_reset_with_shdr32(const char* shstr, elf_section* section, Elf32_Shdr* shdr) {
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
    section->name           = &shstr[shdr->sh_name];
}

void elf_section_reset_with_shdr64(const char* shstr, elf_section* section, Elf64_Shdr* shdr) {
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
    section->name           = &shstr[shdr->sh_name];
}

void elf_segment_reset_with_phdr32(elf_segment *segment, Elf32_Phdr *phdr) {
    assert(segment != NULL);
    assert(phdr != NULL);
    segment->p_type        = (uint32_t)phdr->p_type;
    segment->p_flags       = (uint32_t)phdr->p_flags;
    segment->p_offset      = (uint64_t)phdr->p_offset;
    segment->p_vaddr       = (uint64_t)phdr->p_vaddr;
    segment->p_paddr       = (uint64_t)phdr->p_paddr;
    segment->p_filesz      = (uint64_t)phdr->p_filesz;
    segment->p_memsz       = (uint64_t)phdr->p_memsz;
    segment->p_align       = (uint64_t)phdr->p_align;
}

void elf_segment_reset_with_phdr64(elf_segment *segment, Elf64_Phdr *phdr) {
    assert(segment != NULL);
    assert(phdr != NULL);
    segment->p_type        = (uint32_t)phdr->p_type;
    segment->p_flags       = (uint32_t)phdr->p_flags;
    segment->p_offset      = (uint64_t)phdr->p_offset;
    segment->p_vaddr       = (uint64_t)phdr->p_vaddr;
    segment->p_paddr       = (uint64_t)phdr->p_paddr;
    segment->p_filesz      = (uint64_t)phdr->p_filesz;
    segment->p_memsz       = (uint64_t)phdr->p_memsz;
    segment->p_align       = (uint64_t)phdr->p_align;
}

void elf_dynamic_reset_with_dyn32(elf_dynamic *dynamic, Elf32_Dyn* dyn) {
    assert(dynamic != NULL);
    assert(dyn != NULL);
    dynamic->d_tag = (int64_t)dyn->d_tag;
    dynamic->d_val = (uint64_t)dyn->d_un.d_val;
}
void elf_dynamic_reset_with_dyn64(elf_dynamic *dynamic, Elf64_Dyn* dyn) {
    assert(dynamic != NULL);
    assert(dyn != NULL);
    dynamic->d_tag = (int64_t)dyn->d_tag;
    dynamic->d_val = (uint64_t)dyn->d_un.d_val;
}

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

void elf_reloc_reset_with_rel32(elf_reloc * reloc, const Elf32_Rel * rel) {
    assert(rel != NULL);
    reloc->r_offset      = (addr_t)rel->r_offset;
    reloc->r_info        = (uint64_t)rel->r_info;
    reloc->r_addend      = 0;
    reloc->r_elf_class   = ELFCLASS32;
    reloc->r_use_rela    = false;
}
void elf_reloc_reset_with_rel64(elf_reloc * reloc, const Elf64_Rel * rel) {
    assert(rel != NULL);
    reloc->r_offset      = (addr_t)rel->r_offset;
    reloc->r_info        = (uint64_t)rel->r_info;
    reloc->r_addend      = 0;
    reloc->r_elf_class   = ELFCLASS64;
    reloc->r_use_rela    = false;
}
void elf_reloc_reset_with_rela32(elf_reloc * reloc, const Elf32_Rela * rela) {
    assert(rela != NULL);
    reloc->r_offset      = (addr_t)rela->r_offset;
    reloc->r_info        = (uint64_t)rela->r_info;
    reloc->r_addend      = (int64_t)rela->r_addend;
    reloc->r_elf_class   = ELFCLASS32;
    reloc->r_use_rela    = true;
}
void elf_reloc_reset_with_rela64(elf_reloc * reloc, const Elf64_Rela * rela) {
    assert(rela != NULL);
    reloc->r_offset      = (addr_t)rela->r_offset;
    reloc->r_info        = (uint64_t)rela->r_info;
    reloc->r_addend      = (int64_t)rela->r_addend;
    reloc->r_elf_class   = ELFCLASS64;
    reloc->r_use_rela    = true;
}

int elf_reloc_get_symbol_index(elf_reloc * reloc) {
    if (reloc->r_elf_class == ELFCLASS32) {
        return ELF32_R_SYM(reloc->r_info);
    } else if (reloc->r_elf_class == ELFCLASS64) {
        return ELF64_R_SYM(reloc->r_info);
    }
    return -1;
}
int elf_reloc_get_symbol_type(elf_reloc * reloc) {
    if (reloc->r_elf_class == ELFCLASS32) {
        return ELF32_R_TYPE(reloc->r_info);
    } else if (reloc->r_elf_class == ELFCLASS64) {
        return ELF64_R_TYPE(reloc->r_info);
    }
    return -1;
}
