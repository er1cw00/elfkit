#pragma once

#include <model/elf_hash_tab.h>

class elf_gnu_hash_tab : public elf_hash_tab {
public:
    elf_gnu_hash_tab(uint8_t elf_class,
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
    virtual ~elf_gnu_hash_tab() {}    

    virtual uint32_t get_hash_code(const char* name);
    virtual size_t get_symbol_nums();
    virtual bool find_symbol_by_name(elf_symbol_tab* sym_tab, const char* name, elf_symbol* symbol);
    virtual bool find_symbol_by_addr(elf_symbol_tab* sym_tab, const addr_t addr, elf_symbol* symbol); 
    virtual void dump_hash_table();
    
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