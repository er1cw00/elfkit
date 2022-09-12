#pragma once

#include <common/elf_common.h>
#include <common/elf_string_list.h>

class elf_symbol;
class elf_reader; 
class elf_hash_tab;
class elf_reloc_tab;
class elf_string_tab;
class elf_symbol_tab;
class elf_func_array;

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

    const bool is_use_gnu_hash() {return this->m_is_gnu_hash;}
    const bool is_use_rela() {return this->m_is_use_rela;}

    virtual bool load() = 0;
    virtual bool get_symbol_by_addr(const addr_t addr, elf_symbol* symbol);
    virtual bool get_symbol_by_name(const char* name, elf_symbol* symbol);

    virtual elf_string_tab* get_shstr_tab() {
        return this->m_shstr_tab;
    }
    virtual elf_string_tab* get_str_tab() {
        return this->m_str_tab;
    }
    virtual elf_string_list& get_needed_list() {
        return this->m_needed_list;
    }
    virtual elf_symbol_tab* get_symbol_tab() {
        return this->m_sym_tab;
    }
    virtual elf_hash_tab* get_sysv_hash_tab() {
        return this->m_sysv_hash_tab;
    }
    virtual elf_hash_tab* get_gnu_hash_tab() {
        return this->m_gnu_hash_tab;
    }
    virtual elf_reloc_tab* get_plt_tab() {
        return this->m_plt_tab;
    }
    virtual elf_reloc_tab* get_rel_tab() {
        return this->m_rel_tab;
    }
    virtual elf_reloc_tab* get_relr_tab() {
        return this->m_relr_tab;
    }
protected:
    bool _check_mem_range(addr_t offset, size_t size, size_t alignment);

protected:

    int                 m_fd;
    std::string         m_soname;
    std::string         m_sopath;
    size_t              m_file_size;
    uint8_t             m_elf_class;

    size_t              m_load_size;
    addr_t              m_load_bias;
    
    bool                m_is_gnu_hash;
    bool                m_is_use_rela;

    elf_string_tab*     m_shstr_tab;
    elf_string_tab*     m_str_tab;

    elf_hash_tab*       m_sysv_hash_tab;
    elf_hash_tab*       m_gnu_hash_tab;

    elf_symbol_tab*     m_sym_tab;

    addr_t              m_init_func;
    addr_t              m_finit_func;
    elf_func_array*     m_init_array;
    elf_func_array*     m_finit_array;
    elf_func_array*     m_preinit_array;
    
    elf_reloc_tab*      m_plt_tab;
    elf_reloc_tab*      m_rel_tab;
    elf_reloc_tab*      m_relr_tab;

    elf_string_list     m_needed_list;
};