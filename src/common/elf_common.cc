#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string>
#include <common/elf.h>
#include <common/elf_log.h>
#include <common/elf_common.h>

static elf_logger_t __elf_logger = NULL;

extern "C"
void elf_set_logger(elf_logger_t logger) {
    __elf_logger = logger;
}

void elf_log_vwrite(const int level, const char* fmt, ...) {
    static char line[4096];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, sizeof(line), fmt, ap);
    if (__elf_logger != NULL) {
        __elf_logger(level, line);
    } else {
        fprintf(stdout, "%s\n", line);
        fflush(stdout);
    }
    va_end(ap);
}

bool elf_safe_add(off_t* out, off_t a, size_t b) {
    assert(a >= 0);
    if ((uint64_t)(INT64_MAX - a) < b) {
        return false;
    }
    *out = a + b;
    return true;
}

void elf_dump_hex(uint8_t * pbuf, int size) {
    int j = 0;
    fprintf(stderr, "00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F\n");
    fprintf(stderr, "------------------------------------------------\n");
    for (int i = 0; i < size; i += 16) {
        fprintf(stderr, "%02X %02X %02X %02X %02X %02X %02X %02X  ", 
            pbuf[i + 0], pbuf[i + 1], pbuf[i + 2], pbuf[i + 3],
            pbuf[i + 4], pbuf[i + 5], pbuf[i + 6], pbuf[i + 7]);
        fprintf(stderr, "%02X %02X %02X %02X %02X %02X %02X %02X\n", 
            pbuf[i + 8], pbuf[i + 9], pbuf[i + 10], pbuf[i + 11],
            pbuf[i + 12], pbuf[i + 13], pbuf[i + 14], pbuf[i + 15]);
        j += 16;
    }
    for (; j < size; j += 1) {
        fprintf(stderr, "%02X ", pbuf[j]);
    }
    fprintf(stderr, "\n");
    return;
}


void dump_elf_header(const uint8_t* p) {
    static char alpha_tab[17] = "0123456789ABCDEF";
    char buff[EI_NIDENT*3+1];
    uint8_t elf_class = p[EI_CLASS];
    log_info("ELF%d Header :", elf_class == ELFCLASS64 ? 64 : 32);
    for(int i = 0; i < EI_NIDENT; i++) {
        uint8_t ch = p[i];
        buff[i*3 + 0] = alpha_tab[(int)((ch >> 4) & 0x0F)];
        buff[i*3 + 1] = alpha_tab[(int)(ch & 0x0F)];
        buff[i*3 + 2] = ' ';
    }
    buff[EI_NIDENT*3] = '\0';
    log_info("e_ident:     %s",         buff);
    
    if (elf_class == ELFCLASS64) {
        Elf64_Ehdr* ehdr = (Elf64_Ehdr*)p;
        log_info("e_type:      0x%x",       ehdr->e_type);
        log_info("e_machine:   0x%x",       ehdr->e_machine);
        log_info("e_version:   0x%x",       ehdr->e_version);
        log_info("e_entry:     0x%lx",      (unsigned long)ehdr->e_entry);
        log_info("e_phoff:     0x%lx",      (unsigned long)ehdr->e_phoff);
        log_info("e_shoff:     0x%lx",      (unsigned long)ehdr->e_shoff);
        log_info("e_flags:     0x%x",       ehdr->e_flags);
        log_info("e_ehsize:    0x%x",       ehdr->e_ehsize);
        log_info("e_phentsize: 0x%x",       ehdr->e_phentsize);
        log_info("e_phnum:     0x%x",       ehdr->e_phnum);
        log_info("e_shentsize: 0x%x",       ehdr->e_shentsize);
        log_info("e_shnum:     0x%x",       ehdr->e_shnum);
        log_info("e_shstrndx:  0x%x",       ehdr->e_shstrndx);
    } else if (elf_class == ELFCLASS32) {
        Elf32_Ehdr* ehdr = (Elf32_Ehdr*)p;
        log_info("e_type:      0x%x",       ehdr->e_type);
        log_info("e_machine:   0x%x",       ehdr->e_machine);
        log_info("e_version:   0x%x",       ehdr->e_version);
        log_info("e_entry:     0x%lx",      (unsigned long)ehdr->e_entry);
        log_info("e_phoff:     0x%lx",      (unsigned long)ehdr->e_phoff);
        log_info("e_shoff:     0x%lx",      (unsigned long)ehdr->e_shoff);
        log_info("e_flags:     0x%x",       ehdr->e_flags);
        log_info("e_ehsize:    0x%x",       ehdr->e_ehsize);
        log_info("e_phentsize: 0x%x",       ehdr->e_phentsize);
        log_info("e_phnum:     0x%x",       ehdr->e_phnum);
        log_info("e_shentsize: 0x%x",       ehdr->e_shentsize);
        log_info("e_shnum:     0x%x",       ehdr->e_shnum);
        log_info("e_shstrndx:  0x%x",       ehdr->e_shstrndx);
    }
    return;
}

const char * elf_phdr_type_name(int p_type) {
    const char * _labels[] = {
        "PT_NULL",
        "PT_LOAD",
        "PT_DYNAMIC",
        "PT_INTERP",
        "PT_NOTE",
        "PT_SHLIB",
        "PT_PHDR",
        "PT_TLS"
    };
    if (p_type >= 0 && p_type <= 7) {
        return _labels[(int)p_type];
    } else {
        if (p_type == PT_LOOS) {
            return "PT_LOOS";
        } else if (p_type == PT_HIOS) {
            return "PT_HIOS";
        } else if (p_type == PT_LOPROC) {
            return "PT_LOPROC";
        } else if (p_type == PT_HIPROC) {
            return "PT_HIPROC";
        } else if (p_type == PT_GNU_EH_FRAME) {
            return "EH_FRAME";
        }  else if (p_type == PT_GNU_PROPERTY) {
            return "PROPERTY";
        }  else if (p_type == PT_GNU_STACK) {
            return "GNU_STACK";
        } 
    }
    return "UNKNOWN";
}
const char* elf_protection_flag_name(int p_flag) {
    const char * _labels = "XWR";
    static char result[4];
    for (int i = 0; i < 3; i++) {
        if (p_flag & 1) {
            result[i] = _labels[i];
        } else {
            result[i] = '-';
        }
    }
    result[3] = 0;
    return result;
} 
const char* elf_dynamic_tag_name(int d_tag) {
    const static struct dyn_name_map_t {
        const char * dyn_name;
        int dyn_tag;
    } _dyn_name_maps[] = {
        {"NULL",              0},
        {"NEEDED",            1},
        {"PLTRELSZ",          2},
        {"PLTGOT",            3},
        {"HASH",              4},
        {"STRTAB",            5},
        {"SYMTAB",            6},
        {"RELA",              7},
        {"RELASZ",            8},
        {"RELAENT",           9},
        {"STRSZ",             10},
        {"SYMENT",            11},
        {"INIT",              12},
        {"FINI",              13},
        {"SONAME",            14},
        {"RPATH",             15},
        {"SYMBOLIC",          16},
        {"REL",               17},
        {"RELSZ",             18},
        {"RELENT",            19},
        {"PLTREL",            20},
        {"DEBUG",             21},
        {"TEXTREL",           22},
        {"JMPREL",            23},
        {"BIND_NOW",          24},
        {"INIT_ARRAY",        25},
        {"FINI_ARRAY",        26}, 
        {"INIT_ARRAYSZ",      27},
        {"FINI_ARRAYSZ",      28},
        {"RUNPATH",           29},
        {"FLAGS",             30},
        {"PREINIT_ARRAY",     32},
        {"PREINIT_ARRAYSZ",   33},
        {"RELRSZ",            35},
        {"RELR",              36}, 
        {"RELRENT",           37},
        {"LOPROC",            0x70000000},
        {"HIPROC",            0x7fffffff},
        {"LSDESC_PLT",        DT_TLSDESC_PLT},
        {"GNU_HASH",          DT_GNU_HASH},
        {"ANDROID_REL",       DT_ANDROID_REL},
        {"ANDROID_RELSZ",     DT_ANDROID_RELSZ},
        {"LOOS",              DT_LOOS},
        {"HIOS",              DT_HIOS},
        {"VALRNGLO",          DT_VALRNGLO},
        {"VALRNGHI",          DT_VALRNGHI},
        {"ADDRRNGLO",         DT_ADDRRNGLO},
        {"ADDRRNGHI",         DT_ADDRRNGHI},
        {"VERSYM",            DT_VERSYM},
        {"RELACOUNT",         DT_RELACOUNT},
        {"FLAGS_1",           DT_FLAGS_1},
        {"VERDEF",            DT_VERDEF},
        {"VERDEFNUM",         DT_VERDEFNUM},
        {"VERNEED",           DT_VERNEED},
        {"VERNEEDNUM",        DT_VERNEEDNUM},
        {"OLD_DT_HIOS",       OLD_DT_HIOS},
        {"OLD_DT_LOOS",       OLD_DT_LOOS},
        {NULL, 0}
    };
    for(int i = 0; _dyn_name_maps[i].dyn_name != NULL; i++) {
        if (_dyn_name_maps[i].dyn_tag == d_tag) {
            return _dyn_name_maps[i].dyn_name;
        }
    }
    static char _unk_type[32];
    snprintf(_unk_type, 32, "0x%08X", (uint32_t)d_tag);
    return _unk_type;
}

const char * elf_shdr_type_name(int sh_type) {
    const static char * _labels[] = {
        "SHT_NULL",
        "SHT_PROGBITS",
        "SHT_SYMTAB",
        "SHT_STRTAB",
        "SHT_RELA",
        "SHT_HASH",
        "SHT_DYNAMIC",
        "SHT_NOTE",
        "SHT_NOBITS",
        "SHT_REL",
        "SHT_SHLIB",
        "SHT_DYNSYM",
        "SHT_NUM",      //  12
        "UNKNOWN",
        "INIT_ARRAY",    //14
        "FINI_ARRAY", 
        "PREINIT_ARRAY",
        "GROUP",
        "SYMTAB_SHNDX",
        "RELR",      
        "SHT_NUM",      //20
    };
    if (sh_type >= 0 && sh_type <= 20) {
        return _labels[(int)sh_type];
    } else {
        if (sh_type == SHT_LOPROC) {
            return "LOPROC";
        } else if (sh_type == SHT_HIPROC) {
            return "HIPROC";
        } else if (sh_type == SHT_LOUSER) {
            return "LOUSER";
        } else if (sh_type == SHT_HIUSER) {
            return "HIUSER";
        } else if (sh_type == SHT_ANDROID_REL) {
            return "ANDROID_REL";
        } else if (sh_type == SHT_ANDROID_RELA) {
            return "ANDROID_RELA";
        } else if (sh_type == SHT_ANDROID_RELR) {
            return "ANDROID_RELR";
        } else if (sh_type == SHT_LOOS) {
            return "SHT_LOOS";
        } else if (sh_type == SHT_LOOS) {
            return "SHT_LOOS";
        }
    }
    static char _unk_type[32];
    snprintf(_unk_type, 32, "0x%08X", (uint32_t)sh_type);
    return _unk_type;
}

const char * elf_sym_bind_name(uint32_t s_bind) {
    static const char * _labels[] = {"LOCAL", "GLOBAL", "WEAK"};
    if (s_bind > 2) {
        return "UNKNOW";
    }
    return _labels[s_bind];
}

const char * elf_sym_type_name(uint32_t s_type) {
    static const char * _labels[] = {
        "NOTYPE",
        "OBJECT",
        "FUNC",
        "SECTION",
        "FILE",
        "COMMON",
        "TLS"
    };
    if (s_type == STT_GNU_IFUNC) {
        return "IFUNC";
    } else if (s_type > 6) {
        return "UNKNOW";
    }
    return _labels[s_type];
}

const char * elf_reloc_stype_name(uint16_t em, uint32_t r_type) {
    struct reloc_stype {
        const uint32_t val; 
        const char * name;
    }; 
    static reloc_stype _arm32_stype[] = {
        {R_ARM_NONE,         "R_ARM_NONE"},
        {R_ARM_JUMP_SLOT,    "R_ARM_JUMP_SLOT"},
        {R_ARM_ABS32,        "R_ARM_ABS32"},
        {R_ARM_ABS16,        "R_ARM_ABS16"},
        {R_ARM_ABS12,        "R_ARM_ABS12"},
        {R_ARM_THM_ABS5,     "R_ARM_THM_ABS5"},
        {R_ARM_ABS8,         "R_ARM_ABS8"},
        {R_ARM_GLOB_DAT,     "R_ARM_GLOB_DAT"},
        {R_ARM_RELATIVE,     "R_ARM_RELATIVE"},
        {R_ARM_IRELATIVE,    "R_ARM_IRELATIVE"},
        {R_ARM_COPY,         "R_ARM_COPY"},
        {R_ARM_TLS_DTPMOD32, "R_ARM_TLS_DTPMOD32"},
        {R_ARM_TLS_DTPOFF32, "R_ARM_TLS_DTPOFF32"},     
        {R_ARM_TLS_TPOFF32,  "R_ARM_TLS_TPOFF32"},      
        {R_ARM_TLS_DESC,     "R_ARM_TLS_DESC"},
    };
    static reloc_stype _arm64_stype[] = {
        {R_AARCH64_NONE,        "R_AARCH64_NONE"},
        {R_AARCH64_JUMP_SLOT,   "R_AARCH64_JUMP_SLOT"},
        {R_AARCH64_ABS64,       "R_AARCH64_ABS64"},
        {R_AARCH64_ABS32,       "R_AARCH64_ABS32"},
        {R_AARCH64_ABS16,       "R_AARCH64_ABS16"},
        {R_AARCH64_GLOB_DAT,    "R_AARCH64_GLOB_DAT"},
        {R_AARCH64_RELATIVE,    "R_AARCH64_RELATIVE"},
        {R_AARCH64_IRELATIVE,   "R_AARCH64_IRELATIVE"},
        {R_AARCH64_COPY,        "R_AARCH64_COPY"},
        {R_AARCH64_TLS_DTPMOD,  "R_AARCH64_TLS_DTPMOD"},
        {R_AARCH64_TLS_DTPREL,  "R_AARCH64_TLS_DTPREL"},
        {R_AARCH64_TLS_TPREL,   "R_AARCH64_TLS_TPREL"},
        {R_AARCH64_TLSDESC,     "R_AARCH64_TLSDESC"},
    };
    static reloc_stype _x86_stype[] = {
        {R_386_NONE,         "R_386_NONE"},
        {R_386_PC32,         "R_386_PC32"},
        {R_386_JMP_SLOT,     "R_386_JMP_SLOT"},
        {R_386_32,           "R_386_32"},
        {R_386_GLOB_DAT,     "R_386_GLOB_DAT"},
        {R_386_RELATIVE,     "R_386_RELATIVE"},
        {R_386_IRELATIVE,    "R_386_IRELATIVE"},
        {R_386_COPY,         "R_386_COPY"},
        {R_386_TLS_DTPMOD32, "R_386_TLS_DTPMOD32"},
        {R_386_TLS_DTPOFF32, "R_386_TLS_DTPOFF32"},
        {R_386_TLS_TPOFF32,  "R_386_TLS_TPOFF32"},
        {R_386_TLS_DESC,     "R_386_TLS_DESC"},
    };
    static reloc_stype _x64_stype[] = {
        {R_X86_64_NONE,       "R_X86_64_NONE"},
        {R_X86_64_PC32,       "R_X86_64_PC32"},
        {R_X86_64_JUMP_SLOT,  "R_X86_64_JUMP_SLOT"},
        {R_X86_64_64,         "R_X86_64_64"},
        {R_X86_64_GLOB_DAT,   "R_X86_64_GLOB_DAT"},
        {R_X86_64_RELATIVE,   "R_X86_64_RELATIVE"},
        {R_X86_64_IRELATIVE,  "R_X86_64_IRELATIVE"},
        {R_X86_64_COPY,       "R_X86_64_COPY"},
        {R_X86_64_DTPMOD64,   "R_X86_64_DTPMOD64"},
        {R_X86_64_DTPOFF64,   "R_X86_64_DTPOFF64"},
        {R_X86_64_TPOFF64,    "R_X86_64_TPOFF64"},
        {R_X86_64_TLSDESC,    "R_X86_64_TLSDESC"},
    };
    reloc_stype *stype = NULL;
    size_t num = 0;
    if (em == EM_ARM) {
        stype = _arm32_stype;
        num = sizeof(_arm32_stype) / sizeof(reloc_stype);
    } else if (em == EM_AARCH64) {
        stype = _arm64_stype;
        num = sizeof(_arm64_stype) / sizeof(reloc_stype);
    } else if (em == EM_386 || em == EM_486) {
        stype = _x86_stype;
        num = sizeof(_x86_stype) / sizeof(reloc_stype);
    }  else if (em == EM_X86_64) {
        stype = _x64_stype;
        num = sizeof(_x64_stype) / sizeof(reloc_stype);
    }
    for (int i = 0; i < num; i++) {
        if (stype[i].val == r_type) {
            return stype[i].name;
        }
    }
    return "UNKNOW";
}