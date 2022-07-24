#pragma once

#include "elf_common.h"

#include <string>
#include <vector>

class elf_reader;
class elf_section;
class elf_segment;
class elf_symbol;
class elf_reloc;
class elf_symbol_tab;
class elf_string_tab;
class elf_hash_tab;
class gnu_hash_tab;
class elf_func_array;
class hash_tab;

typedef std::vector<std::string> elf_needed_list_t;
typedef std::vector<elf_reloc*> elf_reloc_list_t;

class elf_image {
public:
    elf_image(elf_reader & reader);
    virtual ~elf_image();

    const int    get_fd() {return this->m_fd;}
    const char * get_soname() {return m_soname.c_str();}
    const char * get_sopath() {return m_sopath.c_str();}
    const size_t get_file_size() {return this->m_file_size;}

    const size_t get_load_size() {return this->m_load_size;}
    const addr_t get_load_bias() {return this->m_load_bias;}
    const uint8_t get_elf_class() {return this->m_elf_class;}

    const bool is_gnu_hash() {return this->m_is_gnu_hash;}

public:

    virtual bool load() = 0;
    virtual elf_section * get_elf_section_by_index(const int index) = 0;
    virtual elf_section * get_elf_section_by_type(const int type) = 0;
    virtual elf_segment * get_elf_segment_by_index(const int index) = 0;
    virtual elf_segment * get_elf_segment_by_type(const int type) = 0;

    virtual elf_string_tab* get_section_string_tab() {
        return this->m_sh_str_tab;
    }
    virtual elf_string_tab * get_string_tab() {
        return this->m_str_tab;
    }
    virtual elf_needed_list_t & get_needed_list() {
        return this->m_needed_list;
    }
    virtual elf_symbol_tab * get_symbol_tab() {
        return this->m_sym_tab;
    }
    virtual hash_tab * get_elf_hash_tab() {
        return this->m_elf_hash_tab;
    }
    virtual hash_tab * get_gnu_hash_tab() {
        return this->m_gnu_hash_tab;
    }
    virtual bool is_use_gnu_hash() {
        return m_is_gnu_hash;
    }
protected:
    bool _check_mem_range(addr_t offset, size_t size, size_t alignment);

protected:
    int                 m_fd;
    size_t              m_file_size;
    std::string         m_soname;
    std::string         m_sopath;

    size_t              m_load_size;
    addr_t              m_load_bias;
    uint8_t             m_elf_class;

    elf_string_tab*     m_sh_str_tab;
    elf_string_tab*     m_str_tab;

    hash_tab*           m_elf_hash_tab;
    hash_tab*           m_gnu_hash_tab;

    elf_symbol_tab*     m_sym_tab;

    bool                m_is_gnu_hash;
    bool                m_is_use_rela;

    addr_t              m_init_func;
    addr_t              m_finit_func;
    elf_func_array*     m_init_array;
    elf_func_array*     m_finit_array;
    elf_func_array*     m_preinit_array;

    elf_needed_list_t   m_needed_list;
    elf_reloc_list_t    m_plt_list;
    elf_reloc_list_t    m_rel_list;
};
