#pragma once

#include "elf.h"

class elf_string_tab {
    public:
    elf_string_tab(const char *tab, const size_t tab_size) {
        this->m_str_tab = (char*)tab;
        this->m_tab_size = tab_size;
    }
    const char * get_string(const int index) {
        if (m_str_tab != NULL && m_tab_size > 0 && index < (int)m_tab_size) {
            return &m_str_tab[index];
        }
        return NULL;
    }
protected:
    
    char * m_str_tab;
    size_t m_tab_size;
};