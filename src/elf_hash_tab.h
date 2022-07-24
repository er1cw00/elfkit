#pragma once

#include "elf_common.h"


class elf_symbol;
class elf_symbol_tab;

class hash_tab {
public:
    hash_tab() {;}
    virtual ~hash_tab() { return; }
    virtual uint32_t get_hash_code(const char * name) = 0 ;
    virtual size_t get_symbol_nums() = 0; 
    virtual elf_symbol* find_symbol_by_name(elf_symbol_tab* sym_tab, const char* name) = 0;
    virtual elf_symbol* find_symbol_by_addr(elf_symbol_tab* sym_tab, const addr_t addr) = 0;
protected:
    bool symbol_matches_soaddr(elf_symbol* sym, addr_t soaddr);
};

class elf_hash_tab : public hash_tab {
public:
    elf_hash_tab(uint32_t nbucket, uint32_t nchain, uint32_t* bucket, uint32_t* chain) {
        this->m_nbucket = nbucket;
        this->m_nchain = nchain;
        this->m_bucket = bucket;
        this->m_chain = chain;
    }
    virtual ~elf_hash_tab() {}

    virtual uint32_t get_hash_code(const char* name);
    virtual size_t get_symbol_nums(); 
    virtual elf_symbol* find_symbol_by_name(elf_symbol_tab* sym_tab, const char* name);
    virtual elf_symbol* find_symbol_by_addr(elf_symbol_tab* sym_tab, const addr_t addr); 
protected:
    uint32_t     m_nbucket;
    uint32_t     m_nchain;
    uint32_t*    m_bucket;
    uint32_t*    m_chain;
};

class gnu_hash_tab : public hash_tab {
public:
    gnu_hash_tab(uint8_t elf_class,
                 uint32_t nbucket,
                 uint32_t symndx,
                 uint32_t maskwords,
                 uint32_t shift2,
                 uint32_t* bucket,
                 uint32_t* chain,
                 addr_t bloom_filter) {
        this->m_elf_class           = elf_class;
        this->m_gnu_nbucket         = nbucket;
        this->m_gnu_symndx          = symndx;
        this->m_gnu_maskwords       = maskwords;
        this->m_gnu_shift2          = shift2;
        this->m_gnu_bucket          = bucket;
        this->m_gnu_chain           = chain;
        this->m_gnu_bloom_filter    = bloom_filter;
        caculate_symbol_nums();
    } 
    virtual ~gnu_hash_tab() {}    

    virtual uint32_t get_hash_code(const char* name);
    virtual size_t get_symbol_nums();
    virtual elf_symbol* find_symbol_by_name(elf_symbol_tab* sym_tab, const char* name);
    virtual elf_symbol* find_symbol_by_addr(elf_symbol_tab* sym_tab, const addr_t addr); 
protected:
    void caculate_symbol_nums();

protected:
    uint8_t     m_elf_class;
    uint32_t    m_gnu_nbucket;
    uint32_t    m_gnu_symndx;
    uint32_t    m_gnu_maskwords;
    uint32_t    m_gnu_shift2;
    uint32_t*   m_gnu_bucket;
    uint32_t*   m_gnu_chain;
    addr_t      m_gnu_bloom_filter;
    size_t      m_symbol_nums;
};