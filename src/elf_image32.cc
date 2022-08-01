
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>

#include "elf_log.h"
#include "elf_reader.h"
#include "elf_image32.h"
#include "elf_section.h"
#include "elf_segment.h"

elf_image32::elf_image32(elf_reader & reader) : elf_image(reader) {
    log_dbg("elf_image32 ctor, this: %p\n", this);
}

elf_image32::~elf_image32() {
    log_dbg("elf_image32 dtor, this: %p\n", this);
}

bool elf_image32::load() {
    return false;
}

bool elf_image32::get_elf_section_by_index(const int index, elf_section* section) {
    return false;
}

bool elf_image32::get_elf_section_by_type(const int type, elf_section* section) {
    return false;
}

bool elf_image32::get_elf_segment_by_index(const int index, elf_segment* segment) {
    return false;
}
bool elf_image32::get_elf_segment_by_type(const int type, elf_segment* segment) {
    return false;
}
