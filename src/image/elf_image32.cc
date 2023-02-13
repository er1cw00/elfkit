#include <vector>
#include <string>

#include <common/elf.h>
#include <common/elf_log.h>
#include <common/elf_common.h>
#include <model/elf_type.h>
#include <model/elf_hash_tab.h>
#include <model/elf_sysv_hash_tab.h>
#include <model/elf_gnu_hash_tab.h>
#include <model/elf_reloc_tab.h>
#include <model/elf_symbol_tab.h>
#include <model/elf_string_tab.h>
#include <model/elf_func_array.h>
#include <image/elf_image32.h>

elf_image32::elf_image32(elf_reader* reader) : elf_image(reader)  {
}

elf_image32::~elf_image32() {
}

bool elf_image32::load() {
     this->m_ehdr = (Elf32_Ehdr*)this->get_load_bias();

    if (this->m_ehdr->e_type != ET_EXEC && this->m_ehdr->e_type != ET_DYN) {
        return false;
    }
    Elf32_Phdr * dyn_phdr = _find_segment_by_type(PT_DYNAMIC);
    if (dyn_phdr == NULL) {
        return  false;
    }
    this->m_dynamic             = (void*)(this->get_load_bias() + dyn_phdr->p_vaddr);
    this->m_dynamic_size        = (size_t)(dyn_phdr->p_memsz/sizeof(Elf32_Dyn));

    Elf32_Sym* symtab           = NULL;
    size_t sym_size             = 0;

    const char* strtab          = NULL;
    size_t strtab_size          = 0;


    addr_t plt_offset           = NULL;
    size_t plt_size             = 0;
    addr_t rel_offset           = NULL;
    size_t rel_size             = 0;
    size_t rel_count            = 0;
    size_t rel_entry_size       = 0;

    addr_t relr_offset          = NULL;
    size_t relr_size            = 0;

    addr_t init_array           = 0;
    addr_t finit_array          = 0;
    addr_t preinit_array        = 0;
    size_t init_array_count     = 0;
    size_t finit_array_count    = 0;
    size_t preinit_array_count  = 0;


    std::vector<int> needed_list;

    for (Elf64_Dyn* d = (Elf64_Dyn*)this->m_dynamic; d->d_tag != DT_NULL; ++d) {
        switch(d->d_tag) {
            case DT_SONAME:
                break;
        }
    }
}
void elf_image32::unload() {
    return;
}

Elf32_Phdr* elf_image32::_find_segment_by_type(const uint32_t type) {
    Elf32_Phdr* target = NULL;
    Elf32_Phdr* phdr = (Elf32_Phdr*)this->m_reader->get_phdr_base();
    for(int i = 0; i < this->m_ehdr->e_phnum; i += 1) {
        if(phdr[i].p_type == type) {
            target = phdr + i;
            break;
        }
    }
    return target;
}