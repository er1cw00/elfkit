
#include "elf.h"
#include "elf_log.h"
#include "elf_hash_tab.h"
#include "elf_symbol_tab.h"


bool hash_tab::_symbol_matches_soaddr(elf_symbol* sym, addr_t soaddr) {
    // Skip TLS symbols. A TLS symbol's value is relative to the start of the TLS segment rather than
    // to the start of the solib. The solib only reserves space for the initialized part of the TLS
    // segment. (i.e. .tdata is followed by .tbss, and .tbss overlaps other sections.)
    return sym->st_shndx != SHN_UNDEF &&
           ELF_ST_TYPE(sym->st_info) != STT_TLS &&
           soaddr >= sym->st_value &&
           soaddr < sym->st_value + sym->st_size;
}

uint32_t elf_hash_tab::get_hash_code(const char * name) {
    const uint8_t *tmp = (const uint8_t *) name;
    uint32_t h = 0, g;
    while (*tmp) {
        h = (h << 4) + *tmp++;
        g = h & 0xf0000000;
        h ^= g;
        h ^= g >> 24;
    }
    return h;
}

bool elf_hash_tab::find_symbol_by_addr(elf_symbol_tab* sym_tab, const addr_t addr, elf_symbol* symbol) {

    if (!symbol) {
        return false;
    }
    for (size_t i = 0; i < m_nchain; ++i) {
        if (sym_tab->get_symbol(i, symbol) && 
            _symbol_matches_soaddr(symbol, addr)) {
            return true;
        }
    }
    return false;
}

bool elf_hash_tab::find_symbol_by_name(elf_symbol_tab* sym_tab, const char * name, elf_symbol* symbol) {
    if (!symbol || !this->m_bucket || !this-> m_chain) {
        return false;
    }
    uint32_t hash = get_hash_code(name);
    uint32_t index = this->m_bucket[hash % this->m_nbucket];

    log_dbg("search sym name(%s), hash(%x), index(%x)\n", name, hash, index);
    for (uint32_t n = index; n != 0; n = m_chain[n]) {
        return sym_tab->get_symbol(n, symbol);
    }
    log_warn("sym name(%s) not found\n", name);
    return false;
}

size_t elf_hash_tab::get_symbol_nums() {
    return (size_t)this->m_chain;
}

uint32_t gnu_hash_tab::get_hash_code(const char * name) {
    uint32_t h = 5381;
    for (unsigned char c = *name; c != '\0'; c = *++name) {
        h = h * 33 + c;
    }
    return h;
}

bool gnu_hash_tab::find_symbol_by_name(elf_symbol_tab* sym_tab, const char * name, elf_symbol* symbol) {
    
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
    if ((1 & (bloom_word >> (hash % bloom_mask_bits)) & (bloom_word >> (h2 % bloom_mask_bits))) == 0) {
        log_dbg("lookup name(%s) NOT Found\n", name);
        return false;
    }

    // bloom test says "probably yes"...
    uint32_t n = this->m_gnu_bucket[hash % this->m_gnu_nbucket];
    if (n == 0) {
        log_dbg("lookup name(%s) NOT Found\n", name);
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

bool gnu_hash_tab::find_symbol_by_addr(elf_symbol_tab* sym_tab, const addr_t addr, elf_symbol* symbol) {
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

size_t gnu_hash_tab::get_symbol_nums() {
    return this->m_symbol_nums + this->m_gnu_symndx;
}

void gnu_hash_tab::_caculate_symbol_nums() {
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

