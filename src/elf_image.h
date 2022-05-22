#pragma once

#include "elf_common.h"
#include "elf_mapped.h"

#include <string>

class elf_image {
public:
    virtual ~elf_image();
    static elf_image * create(const char * sopath);

    bool load();
    const int    get_fd() {return m_fd;}
    const char * get_soname() {return m_soname.c_str();}
    const char * get_sopath() {return m_sopath.c_str();}


protected:
    elf_image();
    virtual bool check_file_range(off64_t offset, size_t size, size_t alignment);
    virtual bool parse_program_headers() = 0;
    virtual bool parse_section_headers() = 0;
    virtual bool parse_sections() = 0;

protected:
    elf_mapped     m_phdr_fragment;
    elf_mapped     m_shdr_fragment;
    elf_mapped     m_dynamic_fragment;
    elf_mapped     m_dynstr_fragment;
    elf_mapped     m_dynsym_fragment;
    elf_mapped     m_strtab_fragment;
    elf_mapped     m_symtab_fragment;
    elf_mapped     m_shstrtab_fragment;

    std::string     m_sopath;
    std::string     m_soname;
    int             m_fd;
    int             m_file_size;
    bool            m_loaded;
    uint64_t        m_base_addr;
};

class elf_image64 : public elf_image {
public:
    virtual ~elf_image64();

protected:
    elf_image64();
    virtual bool parse_program_headers();
    virtual bool parse_section_headers();
    virtual bool parse_sections();

protected:
    Elf64_Ehdr      m_ehdr;

    Elf64_Phdr     *m_phdr;
    Elf64_Shdr      *m_shdr;
    Elf64_Dyn       *m_dynamic;
    Elf64_Sym       *m_dynsym;
    Elf64_Sym       *m_symtab;
    const char      *m_dynstr;
    const char      *m_strtab;
    const char      *m_shstrtab;  
    size_t          m_phdr_num;
    size_t          m_shdr_num;
    size_t          m_dynamic_size;
    size_t          m_dynsym_size;
    size_t          m_symtab_size;
    size_t          m_dynstr_size;
    size_t          m_strtab_size;
    size_t          m_shstrtab_size;
    friend class elf_image;
};

class elf_image32 : public elf_image {
public:
    virtual ~elf_image32();

protected:
    elf_image32(); 
    virtual bool parse_program_headers();
    virtual bool parse_section_headers();
    virtual bool parse_sections();

protected:
    Elf32_Ehdr      m_ehdr;

    Elf32_Phdr      *m_phdr;
    Elf32_Shdr      *m_shdr;
    Elf32_Dyn       *m_dynamic;
    Elf32_Sym       *m_dynsym;
    Elf32_Sym       *m_symtab;
    const char      *m_dynstr;
    const char      *m_strtab;
    const char      *m_shstrtab;  
    size_t          m_phdr_num;
    size_t          m_shdr_num;
    size_t          m_dynamic_size;
    size_t          m_dynsym_size;
    size_t          m_symtab_size;
    size_t          m_dynstr_size;
    size_t          m_strtab_size;
    size_t          m_shstrtab_size;
    friend class elf_image;
};