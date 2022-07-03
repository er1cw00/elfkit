
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
#include "elf_image.h"
#include "elf_image32.h"
#include "elf_image64.h"


elf_image::elf_image(elf_reader & reader) {
    log_dbg("elf_image ctor\n");
    m_fd          = (int)reader.get_fd();
    m_file_size   = reader.get_file_size();
    m_load_bias   = reader.get_load_bias();
    m_load_size   = reader.get_load_size();
    m_elf_class   = reader.get_elf_class();
    m_soname      = reader.get_soname();
    m_sopath      = reader.get_sopath();

    m_elf_hash_tab     = NULL;
    m_gnu_hash_tab     = NULL;

    m_sh_str_tab       = NULL;
    m_str_tab          = NULL;
    m_sym_tab          = NULL;
    m_init_func        = NULL;
    m_finit_func       = NULL;
    m_init_array       = NULL;
    m_finit_array      = NULL;
    m_preinit_array    = NULL;

    m_is_gnu_hash  = false;
    m_is_use_rela  = false;
}

elf_image::~elf_image() {
    log_dbg("elf_image dtor\n");
    if (m_fd != -1) {
        close(m_fd);
        m_fd = -1;
        log_dbg("elf_image close fd\n");
    }
}

bool elf_image::check_mem_range(addr_t offset, size_t size, size_t alignment) {
    off_t range_start;
    off_t range_end;
    off_t mem_end = m_load_bias + m_load_size;
    return offset > 0 &&
        safe_add(&range_start, 0, offset) &&
        safe_add(&range_end, range_start, size) &&
        (range_start < mem_end) &&
        (range_end <= mem_end) &&
        ((offset % alignment) == 0);
}
  

