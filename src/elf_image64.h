#pragma once

#include "elf_image.h"

class elf_image64 : public elf_image {
public:
    elf_image64(elf_reader & reader);
    virtual ~elf_image64();

public:
    virtual bool load();
    virtual elf_section * get_elf_section(const int i);
    virtual elf_segment * get_elf_segment(const int i);
    virtual elf_string_tab * get_string_tab(const char * tab_name);

protected:
    Elf64_Phdr* find_segment_by_type(const uint32_t type);
    Elf64_Shdr* find_section_by_name(const char *sname);
    void create_reloc(std::vector<elf_reloc*>& list, addr_t offset, size_t size, size_t entry_size);
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
