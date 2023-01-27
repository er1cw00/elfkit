#pragma once

#include "elf_common.h"
#pragma once
#include <common/elf_common.h>
#include <string>
#include <vector>

class elf_string_list {
public:
    elf_string_list() {
        m_string_list.clear();
    }
    ~elf_string_list() {}
    void clear() {
        m_string_list.clear();
    }
    void append(const char* p) {
        m_string_list.push_back(p);
    }
    const char* get(const int i) {
        const char* p = NULL;
        if (i < m_string_list.size()) {
            p = m_string_list[i];
        }
        return p;
    }
    const size_t size() {
        return m_string_list.size();
    }
    const bool empty() {
        return m_string_list.empty();
    }
protected:
    typedef std::vector<const char*> string_list_t;
    string_list_t m_string_list;
};
