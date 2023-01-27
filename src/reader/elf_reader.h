#pragma once

#include <string>

#include <common/elf_common.h>
#include <common/elf_mapped.h>
#include <common/elf.h>

class elf_image;
class elf_reader {
public:
    elf_reader() {
        m_fd            = -1;
        m_file_size     = 0;
        m_load_bias     = 0;
        m_load_size     = 0;
        m_file_offset   = 0;
        m_phdr          = NULL;
        m_phdr_num      = 0;
        m_shdr          = NULL;
        m_shdr_num      = 0;
        m_shstr         = NULL;  
        m_shstr_size    = 0;
        m_symstr        = NULL;
        m_symstr_size   = 0;
        m_symtab        = NULL;
        m_symtab_size   = 0;
        m_dyntab        = NULL;
        m_dyntab_size   = 0;
    }
    ~elf_reader() {
        m_fd          = -1;
    }
    bool open(const char* sopath);
    void close();

    elf_image* load();

    const int   get_fd() {return m_fd;}
    const char* get_soname() {return m_soname.c_str();}
    const char* get_sopath() {return m_sopath.c_str();}
    const void* get_elf_header() {return (void*)&m_ehdr;}
    const uint8_t get_elf_class(void) {return m_elf_class;}
    const addr_t get_load_bias() {return m_load_bias;}
    const size_t get_load_size() {return m_load_size;}
    const size_t get_file_size() {return m_file_size;}

    void* get_shdr_base(void) {return m_shdr;}
    size_t get_shdr_num(void) {return m_shdr_num;}

    void* get_phdr_base(void) {return m_phdr;}
    size_t get_phdr_num(void) {return m_phdr_num;}

    const char* get_shstr_base(void) {return m_shstr;}
    size_t get_shstr_size(void) {return m_shstr_size;}
    
    const char* get_symstr_base(void) {return m_symstr;}
    size_t get_symstr_size(void) {return m_symstr_size;}

    void* get_symtab_base(void) {return m_symtab;}
    size_t get_symtab_size(void) {return m_symtab_size;}

    void* get_dyntab_base(void) {return m_dyntab;}
    size_t get_dyntab_size(void) {return m_dyntab_size;}

    void* find_section_by_name(const char *sname);
    void* find_section_by_type(const uint32_t type);

protected:
    bool _check_elf_header(void);
    bool _read_section_headers(void);
    bool _read_segment_headers(void);
    bool _read_section_data(void);
    bool _load_segments(void);
    bool _read_segments(void);

    size_t _get_load_size(void* phdr, size_t phdr_num, addr_t* out_min_vaddr, addr_t* out_max_vaddr);
    size_t _get_min_aligment(void* phdr, size_t phdr_num);
    bool _check_file_range(off_t offset, size_t size, size_t alignment);
    
protected:
    std::string     m_sopath;
    std::string     m_soname;
    int             m_fd;
    size_t          m_file_size;
    size_t          m_file_offset;
    size_t          m_load_size;
    addr_t          m_load_bias;
    elf_mapped      m_phdr_fragment;
    elf_mapped      m_shdr_fragment;

    elf_mapped      m_shstr_fragment;
    elf_mapped      m_symstr_fragment;
    elf_mapped      m_symtab_fragment;
    elf_mapped      m_dyntab_fragment;

    uint8_t         m_elf_class;
    union {
        Elf32_Ehdr  ehdr32;
        Elf64_Ehdr  ehdr64;
    } m_ehdr;

    void*           m_phdr;
    size_t          m_phdr_num;

    void*           m_shdr;
    size_t          m_shdr_num;

    const char*     m_shstr;  
    size_t          m_shstr_size;

    const char*     m_symstr;
    size_t          m_symstr_size;

    void*           m_symtab;
    size_t          m_symtab_size;

    void*           m_dyntab;
    size_t          m_dyntab_size;
};


