#pragma once

#include <string>

#include "elf.h"
#include "elf_common.h"
#include "elf_mapped.h"

class elf_image;
class elf_reader {
public:
    elf_reader();
    ~elf_reader();

    bool open(const char * sopath);
    void close();

    elf_image* load_image();

    const int   get_fd() {return m_fd;}
    const char* get_soname() {return m_soname.c_str();}
    const char* get_sopath() {return m_sopath.c_str();}
    const void* get_elf_header() {return (void*)&m_ehdr;}
    const uint8_t get_elf_class(void) {return m_elf_class;}
    const addr_t get_load_bias() {return m_load_bias;}
    const size_t get_load_size() {return m_load_size;}
    const size_t get_file_size() {return m_file_size;}

    void dump_elf_header(void);
    void dump_program_headers(void);
    void dump_section_headers(void);
    void dump_dynamics(void); 
    void dump_sh_string(void);

protected:

    bool check_file_range(off_t offset, size_t size, size_t alignment);
    bool verify_elf_header(void);
    bool read_program_headers(void);
    bool read_section_headers(void);
    bool read_dynamic_section(void);
    bool load_segment(void);
    bool read_segment(void);
    
    addr_t page_start(addr_t addr, size_t page_size);
    addr_t page_end(addr_t addr, size_t page_size);
    off_t page_offset(addr_t addr, size_t page_size);

    size_t get_load_size(void* phdr, size_t phdr_num, addr_t* out_min_vaddr, addr_t* out_max_vaddr = NULL);
    size_t get_max_aligment(void* phdr, size_t phdr_num);
    size_t get_min_aligment(void* phdr, size_t phdr_num); 
protected:
    std::string     m_sopath;
    std::string     m_soname;
    bool            m_loaded;
    int             m_fd;
    size_t          m_file_size;
    size_t          m_file_offset;
    size_t          m_load_size;
    addr_t          m_load_bias;
    
    elf_mapped      m_phdr_fragment;
    elf_mapped      m_shdr_fragment;
    elf_mapped      m_dynamic_fragment;
    elf_mapped      m_dynstr_fragment;
    elf_mapped      m_dynsym_fragment;
    elf_mapped      m_strtab_fragment;
    elf_mapped      m_symtab_fragment;
    elf_mapped      m_shstrtab_fragment;

    uint8_t         m_elf_class;
    union {
        Elf32_Ehdr  ehdr32;
        Elf64_Ehdr  ehdr64;
    } m_ehdr;

    void*           m_phdr;
    size_t          m_phdr_num;

    void*           m_shdr;
    size_t          m_shdr_num;
    
    void*           m_dynamic;
    size_t          m_dynamic_size;

    void*           m_dynsym;
    size_t          m_dynsym_size;

    void*           m_symtab;
    size_t          m_symtab_size;

    const char*     m_dynstr;
    size_t          m_dynstr_size;

    const char*     m_strtab;
    size_t          m_strtab_size;

    const char*     m_shstrtab;  
    size_t          m_shstrtab_size;
};