#pragma once

#include "elf.h"
#include "elf_log.h"

class elf_func_array {
public:
    elf_func_array(addr_t array, size_t nums, uint8_t elf_class) {
        log_trace("elf_func_array ctor: %p\n", this);
        this->m_func_array = array;
        this->m_func_nums = nums;
        this->m_elf_class = elf_class;
    }
    ~elf_func_array() {
        log_trace("elf_func_array dtor: %p\n", this);
    }
    size_t get_func_nums() {
        return m_func_nums;
    }
    
protected:
    addr_t  m_func_array;
    size_t  m_func_nums;
    uint8_t m_elf_class;
};