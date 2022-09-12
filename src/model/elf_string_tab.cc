
#include <model/elf_string_tab.h>

const char * elf_string_tab::get_string(const int index) {
    if (str_tab != NULL && tab_size > 0 && index < (int)tab_size) {
        return &str_tab[index];
    }
    return NULL;
}