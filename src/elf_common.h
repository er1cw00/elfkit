
#pragma once

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "elf.h"

typedef off_t off64_t;


#define CHECK(predicate)    do { \
                                if (!(predicate)) { \
                                    log_fatal("%s:%d: %s CHECK '" #predicate "' failed", \
                                                __FILE__, __LINE__, __FUNCTION__); \
                                } \
                            } while(0)

bool safe_add(off64_t* out, off64_t a, size_t b);


void dump_hex(uint8_t * pbuf, int size);
