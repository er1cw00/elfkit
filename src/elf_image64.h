#pragma once

#include "elf_image.h"

class elf_image64 : public elf_image {
public:
    elf_image64(elf_reader & reader);
    virtual ~elf_image64();

public:
    virtual bool load();
    virtual bool get_elf_section_by_index(const int index, elf_section* section);
    virtual bool get_elf_section_by_type(const int type, elf_section* section);
    virtual bool get_elf_segment_by_index(const int index, elf_segment* segment);
    virtual bool get_elf_segment_by_type(const int type, elf_segment* segment);

protected:
    Elf64_Phdr* _find_segment_by_type(const uint32_t type);
    Elf64_Shdr* _find_section_by_name(const char *sname);
    Elf64_Shdr* _find_section_by_type(const uint32_t type);

    void _create_reloc(elf_reloc_tab& list, addr_t offset, size_t size, size_t entry_size);
protected:
    Elf64_Ehdr      *m_ehdr;

    Elf64_Phdr      *m_phdr;
    size_t          m_phdr_num;

    Elf64_Shdr      *m_shdr;
    size_t          m_shdr_num;
    
    Elf64_Dyn       *m_dynamic;
    size_t          m_dynamic_size;

    friend class elf_image;
};
