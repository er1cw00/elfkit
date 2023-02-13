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
    Elf32_Phdr* _find_segment_by_type(const uint32_t type);

protected:
    Elf32_Ehdr      *m_ehdr;
    friend class elf_image;
};
