#pragma once

#include "elf_image.h"

class elf_image32 : public elf_image {
public:
    elf_image32(elf_reader & reader);
    virtual ~elf_image32();

public:
    virtual bool load();
    virtual bool get_elf_section_by_index(const int index, elf_section* section);
    virtual bool get_elf_section_by_type(const int type, elf_section* section);
    virtual bool get_elf_segment_by_index(const int index, elf_segment* segment);
    virtual bool get_elf_segment_by_type(const int type, elf_segment* segment);

protected:
    Elf32_Ehdr*     m_ehdr;

    Elf32_Phdr*     m_phdr;
    size_t          m_phdr_num;

    Elf32_Shdr*     m_shdr;
    size_t          m_shdr_num;

    Elf32_Dyn*       m_dynamic;
    size_t          m_dynamic_size;
    
    friend class elf_image;
};