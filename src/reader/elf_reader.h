#pragma once

#include <string>

#include <common/elf_common.h>
#include <common/elf_mapped.h>
#include <common/elf.h>

class elf_image;
class elf_reader {
public:
    elf_reader() {
        m_fd          = -1;
        m_file_size   = 0;
        m_load_bias   = 0;
        m_load_size   = 0;
        m_file_offset = 0;
    }
    ~elf_reader() {
        m_fd          = -1;
    }
    bool open(const char* sopath);
    void close();
    void detach();
    elf_image* load();

    const int   get_fd() {return m_fd;}
    const char* get_soname() {return m_soname.c_str();}
    const char* get_sopath() {return m_sopath.c_str();}
    const void* get_elf_header() {return (void*)&m_ehdr;}
    const uint8_t get_elf_class(void) {return m_elf_class;}
    const addr_t get_load_bias() {return m_load_bias;}
    const size_t get_load_size() {return m_load_size;}
    const size_t get_file_size() {return m_file_size;}

protected:
    bool check_elf_header();
    bool read_segment_headers();
    size_t get_load_size(void* phdr, size_t phdr_num, addr_t* out_min_vaddr, addr_t* out_max_vaddr);
    size_t get_min_aligment(void* phdr, size_t phdr_num);

    bool load_segments(void);
    bool read_segments(void);

    bool check_file_range(off_t offset, size_t size, size_t alignment);
    
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

    uint8_t         m_elf_class;
    union {
        Elf32_Ehdr  ehdr32;
        Elf64_Ehdr  ehdr64;
    } m_ehdr;
    void*           m_phdr;
    size_t          m_phdr_num;

};


