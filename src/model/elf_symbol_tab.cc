
#include <common/elf_common.h>
#include <model/elf_type.h>
#include <model/elf_symbol_tab.h>
#include <model/elf_string_tab.h>

bool elf_symbol_tab::get_symbol(const int i, elf_symbol* symbol) {
    if (symbol) {
        if (m_elf_class == ELFCLASS32) {
            Elf32_Sym* sym = &((Elf32_Sym*)m_sym_tab)[i];
            elf_symbol_reset_with_sym32(symbol, sym);
        } else if (m_elf_class == ELFCLASS64) {
            Elf64_Sym* sym = &((Elf64_Sym*)m_sym_tab)[i];
            elf_symbol_reset_with_sym64(symbol, sym);
        } else {
            return false;
        }
        if (symbol && m_str_tab) {
            const char * name = m_str_tab->get_string(symbol->st_name);
            symbol->sym_name = (char*)name;
            return true;
        }
    }
    return false;
}