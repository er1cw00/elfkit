
#include <common/elf_log.h>
#include <image/elf_image.h>
#include <reader/elf_reader.h>
#include <model/elf_hash_tab.h>

elf_image::elf_image(elf_reader & reader) {
    log_trace("elf_image ctor, this: %p\n", this);
    m_reader = reader;
    // m_file_size         = reader.get_file_size();
    // m_load_bias         = reader.get_load_bias();
    // m_load_size         = reader.get_load_size();
    // m_elf_class         = reader.get_elf_class();
    // m_soname            = reader.get_soname();
    // m_sopath            = reader.get_sopath();
    //reader.detach();
}

elf_image::~elf_image() {
    log_trace("elf_image dtor, this: %p\n", this);
}
bool elf_image::load() {
    return false;
}
void elf_image::unload() {
    m_reader.close();
}
const int elf_image::get_fd() {
    return m_reader.get_fd();
}
const char* elf_image::get_soname() {
    return m_reader.get_soname();
}
const char* elf_image::get_sopath() {
    return m_reader.get_sopath();
}
const size_t elf_image::get_file_size() {
    return m_reader.get_file_size();
}
const size_t elf_image::get_load_size() {
    return m_reader.get_load_size();
}
const addr_t elf_image::get_load_bias() {
    return m_reader.get_load_bias();
}
const uint8_t elf_image::get_elf_class() {
    return m_reader.get_elf_class();
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