#pragma once

#include "elf_image.h"

class elf_image32 : public elf_image {
public:
    elf_image32(elf_reader & reader);
    virtual ~elf_image32();

public:
    virtual bool load();
    virtual elf_section * get_elf_section(const int i);
    virtual elf_segment * get_elf_segment(const int i);
    virtual elf_string_tab * get_string_tab(const char * tab_name);

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