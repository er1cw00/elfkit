#pragma once

#include <common/elf.h>
#include <common/elf_log.h>

class elf_string_tab {
public:
    elf_string_tab(const char *tab, const size_t tab_size) {
        this->str_tab = (char*)tab;
        this->tab_size = tab_size;
    }
    ~elf_string_tab() {
    }

    const char * get_string(const int index);

protected:
    char * str_tab;
    size_t tab_size;
};