#pragma once

#include <common/elf_common.h>
#include <common/elf_string_list.h>
#include <model/elf_type.h>
#include <file/elf_reader.h>

struct elf_symbol;
struct elf_section;
struct elf_segment;

class elf_hash_tab;
class elf_reloc_tab;
class elf_string_tab;
class elf_symbol_tab;
class elf_func_array;

class elf_image {
public:
    elf_image(elf_reader*  reader);
    virtual ~elf_image();

    const int    get_fd() {return m_reader->get_fd();}
    const char * get_soname() { return m_reader->get_soname();}
    const char * get_sopath() {return m_reader->get_sopath();}
    const size_t get_file_size() {return m_reader->get_file_size();}
    const size_t get_load_size() {return m_reader->get_load_size();}
    const addr_t get_load_bias() {return m_reader->get_load_bias();}
    const uint8_t get_elf_class() {return m_reader->get_elf_class();}
    
    const uint16_t get_elf_type();
    const uint16_t get_machine_type();
    const uint8_t get_data_order();

    const size_t get_section_size();
    const size_t get_section_list(elf_section* sections);

    const size_t get_segment_size();
    const size_t get_segment_list(elf_segment* segments);

    const size_t get_dynamic_size();
    const size_t get_dynamic_list(elf_dynamic* dynamic);

    const addr_t get_init_func() {return this->m_init_func;}
    const addr_t get_finit_func() {return this->m_finit_func;}
    elf_func_array* get_init_array() {return this->m_init_array;}
    elf_func_array* get_finit_array() {return this->m_finit_array;}
    elf_func_array* get_preinit_array() {return this->m_preinit_array;}

    const bool is_use_gnu_hash() {return this->m_is_gnu_hash;}
    const bool is_use_rela() {return this->m_is_use_rela;}
    elf_reader* get_reader() {return this->m_reader;}

    virtual bool load();
    virtual void unload();

    virtual bool get_symbol_by_addr(const addr_t addr, elf_symbol* symbol);
    virtual bool get_symbol_by_name(const char* name, elf_symbol* symbol);

    // virtual elf_string_tab* get_shstr_tab() {
    //     return this->m_shstr_tab;
    // }
    virtual elf_string_tab* get_str_tab() {
        return this->m_str_tab;
    }
    virtual elf_string_list& get_needed_list() {
        return this->m_needed_list;
    }
    virtual elf_symbol_tab* get_sym_tab() {
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
    virtual addr_t get_arm_exidx_offset() {
        return (addr_t)NULL;
    }
    virtual size_t get_arm_exidx_count() {
        return 0;
    }
protected:
    bool _check_mem_range(addr_t offset, size_t size, size_t alignment);
    void _create_str_tab(const char* strtab, const size_t strtab_size);
    void _create_needed_list(std::vector<int> & needed_list);
    void _create_func_array(addr_t init_array, size_t init_array_count,
                        addr_t finit_array,  size_t finit_array_count,
                        addr_t preinit_array, size_t preinit_array_count); 
    void _create_reloc_tab(addr_t relr_offset, size_t relr_size,
                        addr_t rel_offset, size_t rel_size,
                        addr_t plt_offset, size_t plt_size,
                        size_t rel_entry_size);
protected:
    elf_reader*         m_reader;
    
    bool                m_is_gnu_hash;
    bool                m_is_use_rela;

    void                *m_dynamic;
    size_t              m_dynamic_size;
//    elf_string_tab*     m_shstr_tab;
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