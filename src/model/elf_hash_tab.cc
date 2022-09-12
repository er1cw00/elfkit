#include <common/elf.h>
#include <model/elf_type.h>
#include <model/elf_hash_tab.h>

bool elf_hash_tab::_symbol_matches_soaddr(elf_symbol* sym, addr_t soaddr) {
    // Skip TLS symbols. A TLS symbol's value is relative to the start of the TLS segment rather than
    // to the start of the solib. The solib only reserves space for the initialized part of the TLS
    // segment. (i.e. .tdata is followed by .tbss, and .tbss overlaps other sections.)
    return sym->st_shndx != SHN_UNDEF &&
           ELF_ST_TYPE(sym->st_info) != STT_TLS &&
           soaddr >= sym->st_value &&
           soaddr < sym->st_value + sym->st_size;
}