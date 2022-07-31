#pragma once

#include "elf_common.h"



class string_list {
public:
    string_list() {}
    ~string_list() {}
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
