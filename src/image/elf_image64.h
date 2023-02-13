#pragma once

#include <image/elf_image.h>

class elf_image64 : public elf_image {
public:
    elf_image64(elf_reader* reader);
    virtual ~elf_image64();

public:
    virtual bool load();
    virtual void unload();
    
protected:
    Elf64_Phdr* _find_segment_by_type(const uint32_t type);
    void _create_symbol_tab(Elf64_Sym* symtab);

protected:
    Elf64_Ehdr      *m_ehdr;
    friend class elf_image;
};
