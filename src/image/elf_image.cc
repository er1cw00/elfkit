
#include <common/elf_log.h>
#include <image/elf_image.h>
#include <file/elf_reader.h>
#include <model/elf_hash_tab.h>
#include <model/elf_type.h>

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
const int elf_image::get_fd() {
    return m_reader->get_fd();
}
const char* elf_image::get_soname() {
    return m_reader->get_soname();
}
const char* elf_image::get_sopath() {
    return m_reader->get_sopath();
}
const size_t elf_image::get_file_size() {
    return m_reader->get_file_size();
}
const size_t elf_image::get_load_size() {
    return m_reader->get_load_size();
}
const addr_t elf_image::get_load_bias() {
    return m_reader->get_load_bias();
}
const uint8_t elf_image::get_elf_class() {
    return m_reader->get_elf_class();
}
const uint16_t elf_image::get_machine_type() {
    if (get_elf_class() == ELFCLASS32) {
        elf32_hdr *hdr = (elf32_hdr*)get_load_bias();
        return hdr->e_machine;
    } else if (get_elf_class() == ELFCLASS64) {
        elf64_hdr *hdr = (elf64_hdr*)get_load_bias();
        return hdr->e_machine;
    }
    return (uint16_t)EM_NONE;
}
const uint8_t elf_image::get_data_order() {
    uint8_t* ident = (uint8_t*)get_load_bias();
    return ident[5];
}

const size_t elf_image::get_section_size() {
    return m_reader->get_shdr_num();
}
const size_t elf_image::get_section_list(elf_section* sections) {
    size_t shnum = m_reader->get_shdr_num();
    if (get_elf_class() == ELFCLASS32) {
        Elf32_Shdr* shdr = (Elf32_Shdr*)m_reader->get_shdr_base();
        if (sections && shdr) {
            for (int i = 0; i < shnum; i++) {
                elf_section_reset_with_shdr32(&sections[i], &shdr[i]);
            }
        }
    } else if (get_elf_class() == ELFCLASS64) {
        Elf64_Shdr* shdr = (Elf64_Shdr*)m_reader->get_shdr_base();
        if (sections && shdr) {
            for (int i = 0; i < shnum; i++) {
                elf_section_reset_with_shdr64(&sections[i], &shdr[i]);
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