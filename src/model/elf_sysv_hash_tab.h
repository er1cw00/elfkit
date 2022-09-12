#pragma once

#include <model/elf_hash_tab.h>

class elf_sysv_hash_tab : public elf_hash_tab {
public:
    elf_sysv_hash_tab(uint32_t nbucket, uint32_t nchain, uint32_t* bucket, uint32_t* chain) {
        this->m_nbucket = nbucket;
        this->m_nchain = nchain;
        this->m_bucket = bucket;
        this->m_chain = chain;
    }
    virtual ~elf_sysv_hash_tab() {}

    virtual uint32_t get_hash_code(const char* name);
    virtual size_t get_symbol_nums(); 
    virtual bool find_symbol_by_name(elf_symbol_tab* sym_tab, const char* name, elf_symbol* symbol);
    virtual bool find_symbol_by_addr(elf_symbol_tab* sym_tab, const addr_t addr, elf_symbol* symbol); 
    virtual void dump_hash_table();

protected:
    uint32_t     m_nbucket;
    uint32_t     m_nchain;
    uint32_t*    m_bucket;
    uint32_t*    m_chain;
};
