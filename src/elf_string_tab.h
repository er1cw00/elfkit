#pragma once

#include "elf.h"
#include "elf_log.h"

struct elf_string_tab {
    elf_string_tab(const char *tab, const size_t tab_size) {
        log_trace("elf_string_tab ctor: %p\n", this);
        this->str_tab = (char*)tab;
        this->tab_size = tab_size;
    }

    ~elf_string_tab() {
        log_trace("elf_string_tab dtor: %p\n", this);
    }

    const char * get_string(const int index) {
        if (str_tab != NULL && tab_size > 0 && index < (int)tab_size) {
            return &str_tab[index];
        }
        return NULL;
    }

    char * str_tab;
    size_t tab_size;
};