#pragma once

#include <image/elf_image.h>

class elf_image32 : public elf_image {
public:
    elf_image32(elf_reader* reader);
    virtual ~elf_image32();

public:
    virtual bool load();
    virtual void unload();

protected:
    void _create_symbol_tab(Elf32_Sym* symtab); 
    Elf32_Phdr* _find_segment_by_type(const uint32_t type);

protected:

    friend class elf_image;
};
