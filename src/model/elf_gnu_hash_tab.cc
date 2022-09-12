#include <string>
#include <common/elf.h>
#include <common/elf_log.h>
#include <common/elf_common.h>
#include <model/elf_type.h>
#include <model/elf_symbol_tab.h>
#include <model/elf_gnu_hash_tab.h>

uint32_t elf_gnu_hash_tab::get_hash_code(const char * name) {
    uint32_t h = 5381;
    for (unsigned char c = *name; c != '\0'; c = *++name) {
        h = h * 33 + c;
    }
    return h;
}

bool elf_gnu_hash_tab::find_symbol_by_name(elf_symbol_tab* sym_tab, const char * name, elf_symbol* symbol) {
    
    if (!symbol || !this->m_gnu_bloom_filter || !this->m_gnu_bucket || !this->m_gnu_chain) {
        return false;
    }
    uint32_t hash = this->get_hash_code(name);
    uint32_t bloom_mask_bits = (m_elf_class == ELFCLASS32 ? 4 : 8) * 8;
    uint32_t h1 = hash % bloom_mask_bits;
    uint32_t h2 = (hash >> this->m_gnu_shift2) % bloom_mask_bits;

    uint32_t word_num = (hash / bloom_mask_bits) & this->m_gnu_maskwords;

    uint64_t bloom_word = 0;
    if (m_elf_class == ELFCLASS32) {
        uint32_t *p = (uint32_t*)this->m_gnu_bloom_filter;
        bloom_word = (uint64_t)(p[word_num]);
    } else if (m_elf_class == ELFCLASS64) {
        uint64_t *p = (uint64_t*)this->m_gnu_bloom_filter;
        bloom_word = (uint64_t)(p[word_num]);
    } else {
        assert(0);
    }

    // test against bloom filter
    printf("hash: %d word_num:%d, bloom_word: %llx, h1: %x, h2: %x\n", hash, word_num, bloom_word, h1,h2);
    if ((1 & (bloom_word >> h1) & (bloom_word >> h2)) == 0) {
        log_dbg("lookup name(%s) NOT Found\n", name);
        return false;
    }

    // bloom test says "probably yes"...
    uint32_t n = this->m_gnu_bucket[hash % this->m_gnu_nbucket];
    if (n == 0) {
        log_dbg(" lookup name(%s) NOT Found\n", name);
        return false;
    }

    do {
        if (sym_tab->get_symbol(n, symbol) && 
                ((this->m_gnu_chain[n] ^ hash) >> 1) == 0 &&
                symbol->sym_name != NULL &&
                strcmp(symbol->sym_name, name) == 0) {
            log_dbg("symbol (%s) Found\n", name);
            return true;
        }
    } while ((this->m_gnu_chain[n++] & 1) == 0);
    return false;
}

bool elf_gnu_hash_tab::find_symbol_by_addr(elf_symbol_tab* sym_tab, const addr_t addr, elf_symbol* symbol) {
    if (!symbol) {
        return false;
    }
    for (size_t i = 0; i < m_gnu_nbucket; ++i) {
        uint32_t n = m_gnu_bucket[i];
        if (n == 0) {
            continue;
        }
        do {
            if (sym_tab->get_symbol(n, symbol) && _symbol_matches_soaddr(symbol, addr)) {
                return true;
            }
        } while ((m_gnu_chain[n++] & 1) == 0);
    }
    return false;
}

size_t elf_gnu_hash_tab::get_symbol_nums() {
    return this->m_symbol_nums + this->m_gnu_symndx;
}

void elf_gnu_hash_tab::dump_hash_table() {
    log_info("dump gnu hash tab: \n");
    log_info("buckets num:    %d\n",    m_gnu_nbucket);
    log_info("symbol index:   %d\n",    m_gnu_symndx);
    log_info("mask words num: %d\n",    m_gnu_maskwords + 1);
    log_info("shift count:    %d\n",    m_gnu_shift2);

    std::string filters = "[";
    if (m_elf_class == ELFCLASS32) {
        uint32_t* p = (uint32_t*)m_gnu_bloom_filter;
        for(int i = 0; i < m_gnu_maskwords; i++) {
            char buf[32];
            uint32_t n = p[i];
            snprintf(buf, sizeof(buf), i == 0 ? "%x" : ",%x", n);
            filters.append(buf);
        }
    } else {
        uint64_t* p = (uint64_t*)m_gnu_bloom_filter;
        for(int i = 0; i < m_gnu_maskwords + 1; i++) {
            char buf[64];
            uint64_t n = p[i];
            snprintf(buf, sizeof(buf), i == 0 ? "%llx" : ",%llx", n);
            filters.append(buf);
        }

    }
    filters.append("]");
    log_info("bloom filter:   %s\n",    filters.c_str());

    std::string buckets = "[";
    for(int i = 0; i < m_gnu_nbucket; i++) {
        char buf[32];
        uint32_t n = m_gnu_bucket[i];
        snprintf(buf, sizeof(buf), i == 0 ? "%d" : ",%d", n);
        buckets.append(buf);
    }
    buckets.append("]");
    log_info("buckets:        %s\n", buckets.c_str());

    std::string chains = "[";
    // for(int i = 0; i < m_gnu_nbucket; i++) {
    //     char buf[32];
    //     uint32_t n = m_gnu_chain[i];
    //     snprintf(buf, 32, i == 0 ? "%d" : ",%d", n);
    //     chains.append(buf);
    // }
    chains.append("]");
    log_info("chains:        %s\n", chains.c_str());

    return;
}
void elf_gnu_hash_tab::_caculate_symbol_nums() {
    size_t total = 0;
    for (int i = 0; i < m_gnu_nbucket; ++i) {
        uint32_t n = m_gnu_bucket[i];
        if (n == 0) {
            continue;
        }
        do {
            total += 1;
        } while ((m_gnu_chain[n++] & 1) == 0);
    }
    this->m_symbol_nums = total;
}
