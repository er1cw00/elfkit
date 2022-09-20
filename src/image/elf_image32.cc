#include <vector>
#include <string>

#include <common/elf.h>
#include <common/elf_log.h>
#include <common/elf_common.h>
#include <model/elf_type.h>
#include <model/elf_hash_tab.h>
#include <model/elf_sysv_hash_tab.h>
#include <model/elf_gnu_hash_tab.h>
#include <model/elf_reloc_tab.h>
#include <model/elf_symbol_tab.h>
#include <model/elf_string_tab.h>
#include <model/elf_func_array.h>
#include <image/elf_image32.h>

elf_image32::elf_image32(elf_reader & reader) : elf_image(reader)  {
    log_trace("elf_image32 ctor, this: %p\n", this);
}

elf_image32::~elf_image32() {
    log_trace("elf_image32 dtor, this: %p\n", this);
}

bool elf_image32::load() {
    return false;
}
void elf_image32::unload() {
    return;
}