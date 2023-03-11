#pragma once

#include <common/elf.h>
#include <common/elf_log.h>

class elf_func_array {
public:
    elf_func_array(addr_t array, size_t count, uint8_t elf_class) {
        this->m_func_array = array;
        this->m_func_count = count;
        this->m_elf_class = elf_class;
    }
    ~elf_func_array() {
    }
    size_t get_func_count() {
        return m_func_count;
    }
    addr_t get_func_array() {
        return m_func_array;
    }
    uint8_t get_elf_class() {
        return m_elf_class;
    }
protected:
    addr_t  m_func_array;
    size_t  m_func_count;
    uint8_t m_elf_class;
};