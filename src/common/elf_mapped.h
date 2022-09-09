#pragma once

#include "elf_common.h"

class elf_mapped {
public:
    elf_mapped();
    ~elf_mapped();

    bool map(int fd, off_t base_offset, size_t elf_offset, size_t size);
    void unmap();
    void*  data() const { return m_data; }
    size_t size() const { return m_size; }
    
private:
    void*  m_map_start;
    size_t m_map_size;
    void*  m_data;
    size_t m_size;
};
