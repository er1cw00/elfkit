#pragma once

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <sys/mman.h>

#if defined __APPLE__
#include <mach/mach.h>
#endif

typedef uint64_t  addr_t;

#define PAGE_START(addr)             ((~(PAGE_SIZE - 1)) & (addr))
#define PAGE_END(addr)               PAGE_START((addr) + (PAGE_SIZE-1))
#define PAGE_OFFSET(x)               ((x) & (PAGE_SIZE - 1))

#define powerof2(x)                  ((((x)-1)&(x))==0)

#define CHECK(predicate)    do { \
                                if (!(predicate)) { \
                                    log_fatal("%s:%d: %s CHECK '" #predicate "' failed", \
                                                __FILE__, __LINE__, __FUNCTION__); \
                                } \
                            } while(0)

const char * elf_phdr_type_name(int p_type);
const char * elf_shdr_type_name(int sh_type);
const char * elf_dynamic_tag_name(int d_tag);

bool elf_safe_add(off_t* out, off_t a, size_t b);
void elf_dump_hex(uint8_t * pbuf, int size);