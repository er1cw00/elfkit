#include "elf.h"
#include "elf_segment.h"
#include <assert.h>

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
