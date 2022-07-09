#pragma once

#include "elf.h"

class elf_func_array {
public:
    elf_func_array(addr_t array, size_t nums, uint8_t elf_class) {
        this->m_func_array = array;
        this->m_func_nums = nums;
        this->m_elf_class = elf_class;
    }
    size_t get_func_nums() {
        return m_func_nums;
    }
protected:
    addr_t  m_func_array;
    size_t  m_func_nums;
    uint8_t m_elf_class;
};