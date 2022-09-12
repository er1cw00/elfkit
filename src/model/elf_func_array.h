#pragma once

#include <common/elf.h>
#include <common/elf_log.h>

class elf_func_array {
public:
    elf_func_array(addr_t array, size_t count, uint8_t elf_class) {
        log_trace("elf_func_array ctor: %p\n", this);
        this->m_func_array = array;
        this->m_func_count = count;
        this->m_elf_class = elf_class;
    }
    ~elf_func_array() {
        log_trace("elf_func_array dtor: %p\n", this);
    }
    size_t get_func_count() {
        return m_func_count;
    }
protected:
    addr_t  m_func_array;
    size_t  m_func_count;
    uint8_t m_elf_class;
};