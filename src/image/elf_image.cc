
#include <common/elf_log.h>
#include <image/elf_image.h>
#include <file/elf_reader.h>
#include <model/elf_hash_tab.h>
#include <model/elf_type.h>
#include <model/elf_func_array.h>
#include <model/elf_string_tab.h>
#include <model/elf_reloc_tab.h>

elf_image::elf_image(elf_reader* reader) {
    m_reader        = reader;
    m_is_gnu_hash   = false;
    m_is_use_rela   = false;
    m_str_tab       = NULL;
    m_sysv_hash_tab = NULL;
    m_gnu_hash_tab  = NULL;
    m_sym_tab       = NULL;
    m_init_func     = NULL;
    m_finit_func    = NULL;
    m_init_array    = NULL;
    m_finit_array   = NULL;
    m_preinit_array = NULL;
    m_plt_tab       = NULL;
    m_rel_tab       = NULL;
    m_relr_tab      = NULL;
}

elf_image::~elf_image() {
}
bool elf_image::load() {
    return false;
}
void elf_image::unload() {
    m_reader->close();
}

const uint16_t elf_image::get_elf_type() {
if (get_elf_class() == ELFCLASS32) {
        Elf32_Ehdr *hdr = (Elf32_Ehdr*)m_reader->get_elf_header();
        return hdr->e_type;
    } else if (get_elf_class() == ELFCLASS64) {
        Elf64_Ehdr *hdr = (Elf64_Ehdr*)m_reader->get_elf_header();
        return hdr->e_type;
    }
    return (uint16_t)ET_NONE;
}
const uint16_t elf_image::get_machine_type() {
    if (get_elf_class() == ELFCLASS32) {
        Elf32_Ehdr *hdr = (Elf32_Ehdr*)m_reader->get_elf_header();
        return hdr->e_machine;
    } else if (get_elf_class() == ELFCLASS64) {
        Elf64_Ehdr *hdr = (Elf64_Ehdr*)m_reader->get_elf_header();
        return hdr->e_machine;
    }
    return (uint16_t)EM_NONE;
}

const uint8_t elf_image::get_data_order() {
    uint8_t* ident = (uint8_t*)m_reader->get_elf_header();
    return ident[5];
}

const size_t elf_image::get_section_size() {
    return m_reader->get_shdr_num();
}
const size_t elf_image::get_section_list(elf_section* sections) {
    size_t shnum = m_reader->get_shdr_num();
    const char * shstr = m_reader->get_shstr_base();
    if (get_elf_class() == ELFCLASS32) {
        Elf32_Shdr* shdr = (Elf32_Shdr*)m_reader->get_shdr_base();
        if (sections && shdr) {
            for (int i = 0; i < shnum; i++) {
                elf_section_reset_with_shdr32(shstr, &sections[i], &shdr[i]);
            }
        }
    } else if (get_elf_class() == ELFCLASS64) {
        Elf64_Shdr* shdr = (Elf64_Shdr*)m_reader->get_shdr_base();
        if (sections && shdr) {
            for (int i = 0; i < shnum; i++) {
                elf_section_reset_with_shdr64(shstr, &sections[i], &shdr[i]);
            }
        }
    }
    return shnum;
}
const size_t elf_image::get_segment_size() {
    return m_reader->get_phdr_num();
}
const size_t elf_image::get_segment_list(elf_segment* segments) {
    size_t phnum = m_reader->get_phdr_num();
    if (get_elf_class() == ELFCLASS32) {
        Elf32_Phdr* phdr = (Elf32_Phdr*)m_reader->get_phdr_base();
        if (segments && phdr) {
            for (int i = 0; i < phnum; i++) {
                elf_segment_reset_with_phdr32(&segments[i], &phdr[i]);
            }
        }
    } else if (get_elf_class() == ELFCLASS64) {
        Elf64_Phdr* phdr = (Elf64_Phdr*)m_reader->get_phdr_base();
        if (segments && phdr) {
            for (int i = 0; i < phnum; i++) {
                elf_segment_reset_with_phdr64(&segments[i], &phdr[i]);
            }
        }
    }
    return phnum;
}
const size_t elf_image::get_dynamic_size() {
    return m_dynamic_size;
}
const size_t elf_image::get_dynamic_list(elf_dynamic* dynamic) {
     size_t dyn_num = m_dynamic_size;
    if (get_elf_class() == ELFCLASS32) {
        Elf32_Dyn * dyn = (Elf32_Dyn*)m_dynamic;
        if (dynamic && dyn) {
            for (int i = 0; i < dyn_num; i++) {
                elf_dynamic_reset_with_dyn32(&dynamic[i], &dyn[i]);
            }
        }
    } else if (get_elf_class() == ELFCLASS64) {
        Elf64_Dyn * dyn = (Elf64_Dyn*)m_dynamic;
        if (dynamic && dyn) {
             for (int i = 0; i < dyn_num; i++) {
                elf_dynamic_reset_with_dyn64(&dynamic[i], &dyn[i]);
            }
        }
    }
    return dyn_num;
}

bool elf_image::get_symbol_by_addr(const addr_t addr, elf_symbol* symbol) {
    elf_hash_tab * hashtab = is_use_gnu_hash() ? m_gnu_hash_tab : m_sysv_hash_tab;
    if (hashtab && m_sym_tab) {
        return hashtab->find_symbol_by_addr(m_sym_tab, addr, symbol);
    }
    return false;
}

bool elf_image::get_symbol_by_name(const char* name, elf_symbol* symbol) {
    elf_hash_tab * hashtab = is_use_gnu_hash() ? m_gnu_hash_tab : m_sysv_hash_tab;
    if (hashtab && m_sym_tab) {
        return hashtab->find_symbol_by_name(m_sym_tab, name, symbol);
    }
    return false;
}

bool elf_image::_check_mem_range(addr_t offset, size_t size, size_t alignment) {
    off_t range_start;
    off_t range_end;
    off_t mem_end = get_load_bias() + get_load_size();
    return offset > 0 &&
        elf_safe_add(&range_start, 0, offset) &&
        elf_safe_add(&range_end, range_start, size) &&
        (range_start < mem_end) &&
        (range_end <= mem_end) &&
        ((offset % alignment) == 0);
}
void elf_image::_create_str_tab(const char* strtab, const size_t strtab_size) {
log_dbg("set str tab: %p, %zd\n", strtab, strtab_size);
    if (strtab && strtab_size > 0) {
        this->m_str_tab = new elf_string_tab(strtab, strtab_size);
    }
}
void elf_image::_create_needed_list(std::vector<int> & needed_list) {
    if (!needed_list.empty() && this->m_str_tab) {
        for(std::vector<int>::iterator itor = needed_list.begin(); itor != needed_list.end(); itor++) {
            const char * name = this->m_str_tab->get_string(*itor);
            if (name != NULL) {
               this->m_needed_list.append(name);
            }
        }
    }
}
void elf_image::_create_func_array(addr_t init_array, size_t init_array_count,
                                     addr_t finit_array,  size_t finit_array_count,
                                     addr_t preinit_array, size_t preinit_array_count) {
    if (init_array != NULL && init_array_count > 0) {
        this->m_init_array = new elf_func_array(init_array, init_array_count, get_elf_class());
    }
    if (finit_array != NULL && finit_array_count > 0) {
        this->m_finit_array = new elf_func_array(finit_array, finit_array_count, get_elf_class());    
    }
    if (preinit_array != NULL && preinit_array_count > 0) {
        this->m_preinit_array = new elf_func_array(preinit_array, preinit_array_count, get_elf_class());    
    }
}

void elf_image::_create_reloc_tab(addr_t relr_offset, size_t relr_size,
                                    addr_t rel_offset, size_t rel_size, 
                                    addr_t plt_offset, size_t plt_size,
                                    size_t rel_entry_size) {
    if (plt_offset && plt_size > 0) {
        m_plt_tab = new elf_reloc_tab(get_elf_class(), plt_offset, plt_size/rel_entry_size, m_is_use_rela);
    }
    if (rel_offset && rel_size > 0) {
        m_rel_tab = new elf_reloc_tab(get_elf_class(), rel_offset, rel_size/rel_entry_size, m_is_use_rela);
    }
    if (relr_offset && relr_size > 0) {
        log_fatal("not support DT_RELR\n");
        assert(false);
    }
}