#include <model/elf_type.h>
#include <model/elf_symbol_tab.h>
#include <model/elf_sysv_hash_tab.h>

uint32_t elf_sysv_hash_tab::get_hash_code(const char * name) {
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

bool elf_sysv_hash_tab::find_symbol_by_addr(elf_symbol_tab* sym_tab, const addr_t addr, elf_symbol* symbol) {
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

bool elf_sysv_hash_tab::find_symbol_by_name(elf_symbol_tab* sym_tab, const char * name, elf_symbol* symbol) {
    if (!symbol || !this->m_bucket || !this-> m_chain) {
        return false;
    }
    uint32_t hash = get_hash_code(name);
    uint32_t index = this->m_bucket[hash % this->m_nbucket];

    for (uint32_t n = index; n != 0; n = m_chain[n]) {
        return sym_tab->get_symbol(n, symbol);
    }
    return false;
}

size_t elf_sysv_hash_tab::get_symbol_nums() {
    return (size_t)this->m_nchain;
}
void elf_sysv_hash_tab::dump_hash_table() {

}
