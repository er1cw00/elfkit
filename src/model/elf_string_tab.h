#pragma once

#include <common/elf.h>
#include <common/elf_log.h>

class elf_string_tab {
public:
    elf_string_tab(const char *tab, const size_t tab_size) {
        log_trace("elf_string_tab ctor: %p\n", this);
        this->str_tab = (char*)tab;
        this->tab_size = tab_size;
    }
    ~elf_string_tab() {
        log_trace("elf_string_tab dtor: %p\n", this);
    }

    const char * get_string(const int index);

protected:
    char * str_tab;
    size_t tab_size;
};