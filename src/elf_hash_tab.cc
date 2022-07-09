#include "elf_log.h"
#include "elf_hash_tab.h"
#include "elf_symbol_tab.h"


bool hash_tab::symbol_matches_soaddr(elf_symbol* sym, addr_t soaddr) {
    // Skip TLS symbols. A TLS symbol's value is relative to the start of the TLS segment rather than
    // to the start of the solib. The solib only reserves space for the initialized part of the TLS
    // segment. (i.e. .tdata is followed by .tbss, and .tbss overlaps other sections.)
    return sym->get_section_index() != SHN_UNDEF &&
           ELF_ST_TYPE(sym->get_info()) != STT_TLS &&
           soaddr >= sym->get_value() &&
           soaddr < sym->get_value() + sym->get_size();
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
elf_symbol* elf_hash_tab::find_symbol_by_addr(elf_symbol_tab* sym_tab, const addr_t addr) {

    // Search the library's symbol table for any defined symbol which
    // contains this address.
    for (size_t i = 0; i < m_nchain; ++i) {
        elf_symbol* sym = sym_tab->get_symbol(i);
        if (symbol_matches_soaddr(sym, addr)) {
            return sym;
        }
    }
    return NULL;
}

elf_symbol* elf_hash_tab::find_symbol_by_name(elf_symbol_tab* sym_tab, const char * name) {
    if (!this->m_bucket || !this-> m_chain) {
        return NULL;
    }
    uint32_t hash = get_hash_code(name);
    uint32_t index = this->m_bucket[hash % this->m_nbucket];

    log_dbg("search sym name(%s), hash(%x), index(%x)\n", name, hash, index);
    for (uint32_t n = index; n != 0; n = m_chain[n]) {
        elf_symbol* sym = sym_tab->get_symbol(n);
        return sym;
    }
    log_warn("sym name(%s) not found\n", name);
    return NULL;
}

size_t elf_hash_tab::get_symbol_nums() {
    return (size_t)this->m_chain;
}



uint32_t elf_gnu_hash_tab::get_hash_code(const char * name) {
    uint32_t h = 5381;
    for (unsigned char c = *name; c != '\0'; c = *++name) {
        h = h * 33 + c;
    }
    return h;
}

elf_symbol* elf_gnu_hash_tab::find_symbol_by_name(elf_symbol_tab* sym_tab, const char * name) {
    
    if (!this->m_gnu_bloom_filter || !this->m_gnu_bucket || !this->m_gnu_chain) {
        return NULL;
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

    elf_symbol* sym = NULL;
    // test against bloom filter
    if ((1 & (bloom_word >> (hash % bloom_mask_bits)) & (bloom_word >> (h2 % bloom_mask_bits))) == 0) {
        log_dbg("lookup name(%s) NOT Found\n", name);
        return NULL;
    }

    // bloom test says "probably yes"...
    uint32_t n = this->m_gnu_bucket[hash % this->m_gnu_nbucket];
    if (n == 0) {
        log_dbg("lookup name(%s) NOT Found\n", name);
        return NULL;
    }

    do {
        sym = sym_tab->get_symbol(n);
        if (sym != NULL && 
                ((this->m_gnu_chain[n] ^ hash) >> 1) == 0 &&
                strcmp(sym->get_sym_name(), name) == 0) {
            log_dbg("symbol (%s) Found\n", name);
            return sym;
        }
    } while ((this->m_gnu_chain[n++] & 1) == 0);
    return NULL;
}

elf_symbol* elf_gnu_hash_tab::find_symbol_by_addr(elf_symbol_tab* sym_tab, const addr_t addr) {

    for (size_t i = 0; i < m_gnu_nbucket; ++i) {
        uint32_t n = m_gnu_bucket[i];
        if (n == 0) {
            continue;
        }

        do {
            elf_symbol* sym = sym_tab->get_symbol(n);
            if (symbol_matches_soaddr(sym, addr)) {
                return sym;
            }
        } while ((m_gnu_chain[n++] & 1) == 0);
    }
    return NULL;
}

size_t elf_gnu_hash_tab::get_symbol_nums() {
    return this->m_symbol_nums + this->m_gnu_symndx;
}

void elf_gnu_hash_tab::caculate_symbol_nums() {
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

