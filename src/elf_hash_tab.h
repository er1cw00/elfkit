#pragma once

#include "elf.h"

class hash_tab {
public:
    hash_tab() {;}
    virtual ~hash_tab() { return; }
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

protected:
    uint32_t    m_nbucket;
    uint32_t    m_nchain;
    uint32_t*    m_bucket;
    uint32_t*    m_chain;
};

class elf_gnu_hash_tab : public hash_tab {
public:
    elf_gnu_hash_tab(uint32_t nbucket,
                     uint32_t symndx,
                     uint32_t maskwords,
                     uint32_t shift2,
                     uint32_t* bucket,
                     uint32_t* chain,
                     uint64_t bloom_filter) {
        this->m_gnu_nbucket         = nbucket;
        this->m_gnu_symndx          = symndx;
        this->m_gnu_maskwords       = maskwords;
        this->m_gnu_shift2          = shift2;
        this->m_gnu_bucket          = bucket;
        this->m_gnu_chain           = chain;
        this->m_gnu_bloom_filter    = bloom_filter;
    } 
    virtual ~elf_gnu_hash_tab() {}    

protected:
    uint32_t   m_gnu_nbucket;
    uint32_t   m_gnu_symndx;
    uint32_t   m_gnu_maskwords;
    uint32_t   m_gnu_shift2;
    uint32_t*   m_gnu_bucket;
    uint32_t*   m_gnu_chain;
    uint64_t   m_gnu_bloom_filter;
};