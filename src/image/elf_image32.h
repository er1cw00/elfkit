#pragma once

#include <image/elf_image.h>

class elf_image32 : public elf_image {
public:
    elf_image32(elf_reader* reader);
    virtual ~elf_image32();

public:
    virtual bool load();
    virtual void unload();
    virtual addr_t get_arm_exidx_offset();
    virtual size_t get_arm_exidx_count();

protected:
    void _create_symbol_tab(Elf32_Sym* symtab); 
    void _load_arm_exidx();
    Elf32_Phdr* _find_segment_by_type(const uint32_t type);
    
protected:
    addr_t              m_arm_exidx_offset;
    size_t              m_arm_exidx_count;

    friend class elf_image;
};
