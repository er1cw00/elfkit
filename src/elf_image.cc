
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
#include "elf_common.h"

#include "elf_reader.h"
#include "elf_image.h"
#include "elf_image32.h"
#include "elf_image64.h"

#include "elf_section.h"
#include "elf_segment.h"
#include "elf_reloc_tab.h"
#include "elf_hash_tab.h"
#include "elf_string_tab.h"
#include "elf_symbol_tab.h"
#include "elf_func_array.h"



elf_image::elf_image(elf_reader & reader) {
    log_dbg("elf_image ctor\n");
    m_fd                = (int)reader.get_fd();
    m_file_size         = reader.get_file_size();
    m_load_bias         = reader.get_load_bias();
    m_load_size         = reader.get_load_size();
    m_elf_class         = reader.get_elf_class();
    m_soname            = reader.get_soname();
    m_sopath            = reader.get_sopath();

    m_elf_hash_tab      = NULL;
    m_gnu_hash_tab      = NULL;

    m_sh_str_tab        = NULL;
    m_str_tab           = NULL;
    m_sym_tab           = NULL;
    m_init_func         = NULL;
    m_finit_func        = NULL;
    m_init_array        = NULL;
    m_finit_array       = NULL;
    m_preinit_array     = NULL;

    m_is_gnu_hash       = false;
    m_is_use_rela       = false;
}

elf_image::~elf_image() {
    log_dbg("elf_image dtor\n");
    if (m_load_bias) {
        munmap((void*)m_load_bias, m_load_size);
        m_load_bias = NULL;
        m_load_size = 0;
    }
    if (m_fd != -1) {
        close(m_fd);
        m_fd = -1;
        log_dbg("elf_image close fd\n");
    }
    if (m_sh_str_tab) {
        delete m_sh_str_tab;
        m_sh_str_tab = NULL;
    }
    if (m_str_tab) {
        delete m_str_tab;
        m_str_tab = NULL;
    }
    if (m_elf_hash_tab) {
        delete m_elf_hash_tab;
        m_elf_hash_tab = NULL;
    }
    if (m_gnu_hash_tab) {
        delete m_gnu_hash_tab;
        m_gnu_hash_tab = NULL;
    }
    if (m_sym_tab) {
        delete m_sym_tab;
        m_sym_tab = NULL;
    }
    if (m_init_array) {
        delete m_init_array;
        m_init_array = NULL;
    }
    if (m_finit_array) {
        delete m_finit_array;
        m_finit_array = NULL;
    }
    if (m_preinit_array) {
        delete m_preinit_array; 
        m_preinit_array = NULL;
    }

}



bool elf_image::_check_mem_range(addr_t offset, size_t size, size_t alignment) {
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
  

