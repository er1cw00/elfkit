#pragma once

#include "elf_log.h"
#include "elf_common.h"

class elf_symbol;
class elf_symbol_tab;

class hash_tab {
public:
    hash_tab() {
        log_trace("hash_tab ctor: %p\n", this);
    }
    virtual ~hash_tab() { 
        log_trace("hash_tab dtor: %p\n", this);
    }
    virtual uint32_t get_hash_code(const char * name) = 0 ;
    virtual size_t get_symbol_nums() = 0; 
    virtual bool find_symbol_by_name(elf_symbol_tab* sym_tab, const char* name, elf_symbol* symbol) = 0;
    virtual bool find_symbol_by_addr(elf_symbol_tab* sym_tab, const addr_t addr, elf_symbol* symbol) = 0;
protected:
    bool _symbol_matches_soaddr(elf_symbol* sym, addr_t soaddr);
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
    virtual bool find_symbol_by_name(elf_symbol_tab* sym_tab, const char* name, elf_symbol* symbol);
    virtual bool find_symbol_by_addr(elf_symbol_tab* sym_tab, const addr_t addr, elf_symbol* symbol); 

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
        _caculate_symbol_nums();
    } 
    virtual ~gnu_hash_tab() {}    

    virtual uint32_t get_hash_code(const char* name);
    virtual size_t get_symbol_nums();
    virtual bool find_symbol_by_name(elf_symbol_tab* sym_tab, const char* name, elf_symbol* symbol);
    virtual bool find_symbol_by_addr(elf_symbol_tab* sym_tab, const addr_t addr, elf_symbol* symbol); 

protected:
    void _caculate_symbol_nums();

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