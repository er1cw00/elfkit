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

    void _create_reloc(elf_reloc_tab& list, addr_t offset, size_t size, size_t entry_size);

    void _create_str_tab(const char* strtab, const size_t strtab_size);

    void _create_needed_list(std::vector<int> & needed_list);
    void _create_symbol_tab(Elf64_Sym* symtab);
    void _create_reloc_tab(addr_t relr_offset, size_t relr_size,
                            addr_t rel_offset, size_t rel_size,
                            addr_t plt_offset, size_t plt_size,
                            size_t rel_entry_size);
    void _create_func_array(addr_t init_array, size_t init_array_count,
                            addr_t finit_array,  size_t finit_array_count,
                            addr_t preinit_array, size_t preinit_array_count); 

protected:
    Elf64_Ehdr      *m_ehdr;
    
    Elf64_Dyn       *m_dynamic;
    size_t          m_dynamic_size;

    friend class elf_image;
};
