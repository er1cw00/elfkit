
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>

#include "elf_common.h"
#include "elf_log.h"
#include "elf_reader.h"
#include "elf_image64.h"
#include "elf_section.h"
#include "elf_segment.h"
#include "elf_reloc.h"
#include "elf_hash_tab.h"
#include "elf_string_tab.h"
#include "elf_symbol_tab.h"
#include "elf_func_array.h"


elf_image64::elf_image64(elf_reader & reader) : elf_image(reader)  {
    log_dbg("elf_image64 ctor, this: %p\n", this);
}

elf_image64::~elf_image64() {
    log_dbg("elf_image64 dtor, this: %p\n", this);
}

bool elf_image64::load() {

    this->m_ehdr = (Elf64_Ehdr*)this->get_load_bias();
    this->m_shdr = (Elf64_Shdr*)(this->get_load_bias() + this->m_ehdr->e_shoff);
    this->m_phdr = (Elf64_Phdr*)(this->get_load_bias() + this->m_ehdr->e_phoff);
    if (this->m_ehdr->e_type != ET_EXEC && this->m_ehdr->e_type != ET_DYN) {
        return false;
    }
    Elf64_Phdr * dyn_phdr = find_segment_by_type(PT_DYNAMIC);
    if (dyn_phdr == NULL) {
        return  false;
    }
    this->m_dynamic             = (Elf64_Dyn*)(this->get_load_bias() + dyn_phdr->p_vaddr);
    this->m_dynamic_size        = (size_t)(dyn_phdr->p_memsz/sizeof(Elf64_Dyn));

    Elf64_Sym* symtab           = NULL;
    size_t symtab_size          = 0;
    size_t sym_size             = 0;

    char* strtab                = NULL;
    size_t strtab_size          = 0;

    addr_t plt_rel_offset       = NULL;
    size_t plt_rel_size         = 0;
    addr_t rel_offset           = NULL;
    size_t rel_size             = 0;
    size_t rel_entry_size       = 0;

    addr_t relr                 = NULL;
    size_t relr_count           = 0;

    addr_t init_array           = 0;
    addr_t finit_array          = 0;
    addr_t preinit_array        = 0;
    size_t init_array_count     = 0;
    size_t finit_array_count    = 0;
    size_t preinit_array_count  = 0;


    std::vector<int> needed_list;

    for (Elf64_Dyn* d = this->m_dynamic; d->d_tag != DT_NULL; ++d) {
        log_error("d = %p, d[0](tag) = %s d[1](val) = %p\n",
                  d, 
                  elf_dynamic_tag_name(d->d_tag),
                  reinterpret_cast<void*>(d->d_un.d_val));
        switch(d->d_tag) {
            case DT_SONAME:
                break;
            case DT_HASH:
            {
                uint32_t *rawdata = reinterpret_cast<uint32_t *>(this->get_load_bias() + d->d_un.d_ptr);
                uint32_t nbucket  = rawdata[0];
                uint32_t nchain   = rawdata[1];
                uint32_t *bucket  = (uint32_t*)(rawdata + 2);
                uint32_t *chain   = (uint32_t*)(bucket + nbucket);
                sym_size  = (size_t)nchain;
                this->m_elf_hash_tab = new elf_hash_tab(nbucket, nchain, bucket, chain);
                break;
            }
            case DT_GNU_HASH:
            {
                uint32_t* rawdata          = (uint32_t *)(this->get_load_bias() + d->d_un.d_ptr);
                uint32_t gnu_nbucket       = rawdata[0];
                uint32_t gnu_symndx        = rawdata[1];
                uint32_t gnu_maskwords     = rawdata[2];
                uint32_t gnu_shift2        = rawdata[3];
                uint64_t gnu_bloom_filter  = (uint64_t)(this->get_load_bias() + d->d_un.d_ptr + 16);
                uint32_t* gnu_bucket       = (uint32_t *)(gnu_bloom_filter + gnu_maskwords);
                uint32_t* gnu_chain        = (uint32_t *)(gnu_bucket + gnu_nbucket - gnu_symndx);
                if (!powerof2(gnu_maskwords)) {
                    log_error("invalid maskwords for gnu_hash = 0x%x, in \"%s\" expecting power to two\n",
                            gnu_maskwords, get_soname());
                    return false;
                }
                gnu_maskwords -= 1;
                this->m_gnu_hash_tab = new elf_gnu_hash_tab(gnu_nbucket, 
                                                            gnu_symndx,
                                                            gnu_maskwords,
                                                            gnu_shift2,
                                                            gnu_bucket,
                                                            gnu_chain,
                                                            gnu_bloom_filter);
                this->m_is_gnu_hash = true;
                break;
            }
            case DT_STRTAB:
                strtab = (char*)(this->get_load_bias() + d->d_un.d_ptr);
                break;
            case DT_STRSZ:
                strtab_size = (size_t)d->d_un.d_val;
                break;
            case DT_SYMTAB:
                symtab = (Elf64_Sym*)(this->get_load_bias() + d->d_un.d_ptr);
                break;
            case DT_SYMENT:
                if (d->d_un.d_ptr != sizeof(Elf64_Sym)) {
                    log_error("invalid DT_SYMENT: %zd in \"%s\"\n", (size_t)d->d_un.d_ptr, get_soname());
                    exit(0);
                }
                break;
            case DT_NEEDED:
                needed_list.push_back((int)d->d_un.d_val);
                break;
            case DT_INIT:
                this->m_init_func = (addr_t)(this->get_load_bias() + d->d_un.d_ptr);
                break;
            case DT_FINI:
                this->m_finit_func = (addr_t)(this->get_load_bias() + d->d_un.d_ptr);
                break;
            case DT_INIT_ARRAY:
                init_array = (addr_t)(this->get_load_bias() + d->d_un.d_ptr);
                break;
            case DT_INIT_ARRAYSZ:
                init_array_count = (size_t)(d->d_un.d_val) / sizeof(Elf64_Addr);
                break;
            case DT_FINI_ARRAY:
                finit_array = (addr_t)(this->get_load_bias() + d->d_un.d_ptr);
                break;            
            case DT_FINI_ARRAYSZ:
                init_array_count = (size_t)(d->d_un.d_val) / sizeof(Elf64_Addr);
                break;
            case DT_PREINIT_ARRAY:
                preinit_array = (addr_t)(this->get_load_bias() + d->d_un.d_ptr);
                break;
            case DT_PREINIT_ARRAYSZ:
                preinit_array_count = (size_t)(d->d_un.d_val) / sizeof(Elf64_Addr);
                break;
            case DT_PLTREL:
                this->m_is_use_rela = (d->d_un.d_val == DT_RELA);
                break;
            case DT_PLTRELSZ:
                plt_rel_size = (size_t)d->d_un.d_val;
                break;
            case DT_JMPREL:
                plt_rel_offset = (addr_t)(this->get_load_bias() + d->d_un.d_ptr);
                break;
            case DT_REL:
            case DT_RELA:
                rel_offset = (addr_t)(this->get_load_bias() + d->d_un.d_ptr);
                break;
            case DT_RELSZ:
            case DT_RELASZ:
                rel_size = (size_t)d->d_un.d_val;
                break;
            case DT_RELENT:
            case DT_RELAENT:
                rel_entry_size = (size_t)d->d_un.d_val;
                break;
            case DT_RELRSZ:
            case DT_ANDROID_RELRSZ:
                relr_count = (size_t)(d->d_un.d_val / sizeof(Elf64_Relr));
                break;
            case DT_RELR:
            case DT_ANDROID_RELR:
                relr = (addr_t)(this->get_load_bias() + d->d_un.d_ptr);
                break;
            case DT_PLTGOT:
            case DT_DEBUG:
            case DT_RUNPATH:
            default:
                log_warn("unsupport dyn tag: %s\n", elf_dynamic_tag_name(d->d_tag));
                break;
        }   
    }
    Elf64_Shdr * shstrtab_shdr = &this->m_shdr[this->m_ehdr->e_shstrndx];
    if (shstrtab_shdr && check_mem_range(shstrtab_shdr->sh_offset, shstrtab_shdr->sh_size, 4)) {
        this->m_sh_str_tab = new elf_string_tab((const char*)shstrtab_shdr->sh_offset, (size_t)
            shstrtab_shdr->sh_size);
    }

    if (init_array != NULL && init_array_count > 0) {
        this->m_init_array = new elf_func_array(init_array, init_array_count, get_elf_class());
    }
    if (finit_array != NULL && finit_array_count > 0) {
        this->m_finit_array = new elf_func_array(finit_array, finit_array_count, get_elf_class());    
    }
    if (preinit_array != NULL && preinit_array_count > 0) {
        this->m_preinit_array = new elf_func_array(preinit_array, preinit_array_count, get_elf_class());    
    }

    if (strtab && strtab_size > 0) {
        this->m_str_tab = new elf_string_tab(strtab, strtab_size);
    }
    if (!needed_list.empty() && this->m_str_tab) {
        for(std::vector<int>::iterator itor = needed_list.begin(); itor !=needed_list.end(); itor++) {
            const char * name = this->m_str_tab->get_string(*itor);
            log_dbg("needed: %s\n", name);
            if (name != NULL) {
                this->m_needed_list.push_back(name);
            }
        }
    }
    if (symtab) {
        m_sym_tab = new elf_symbol_tab(symtab, this->m_str_tab, is_gnu_hash() ? m_gnu_hash_tab : m_elf_hash_tab);
    }

    assert(rel_entry_size == sizeof(Elf64_Rel) || rel_entry_size == sizeof(Elf64_Rela));
    if (plt_rel_offset && plt_rel_size > 0) {
        create_reloc(m_plt_list, plt_rel_offset, plt_rel_size, rel_entry_size);
    }
    if (rel_offset && rel_size > 0) {
        create_reloc(m_rel_list, rel_offset, rel_size, rel_entry_size);
    }

    return true;
}

void elf_image64::create_reloc(std::vector<elf_reloc*>& list, addr_t offset, size_t size, size_t entry_size) {
    if (m_is_use_rela) {
        assert(entry_size == sizeof(Elf64_Rela));
        size_t count = size / entry_size;
        Elf64_Rela* plt_rela = (Elf64_Rela*)offset;
        for (int i = 0; i < count; i++) {
            elf_reloc * reloc = new elf_reloc(&plt_rela[i]);
            list.push_back(reloc);
        }
    } else {
        assert(entry_size == sizeof(Elf64_Rel));
        size_t count = size / entry_size;
        Elf64_Rel* plt_rel = (Elf64_Rel*)offset;
        for (int i = 0; i < count; i++) {
            elf_reloc * reloc = new elf_reloc(&plt_rel[i]);
            list.push_back(reloc);
        }
    }
}
elf_section * elf_image64::get_elf_section(const int i) {
    if (i < m_shdr_num) {
        elf_section * section = new elf_section(&m_shdr[i]);
        return section;
    }
    return NULL;
}

elf_segment * elf_image64::get_elf_segment(const int i) {
    if (i < m_phdr_num) {
        elf_segment * segment = new elf_segment(&m_phdr[i]);
        return segment;
    }
    return NULL;
}


Elf64_Phdr* elf_image64::find_segment_by_type(const uint32_t type) {
    Elf64_Phdr* target = NULL;
    Elf64_Phdr* phdr = this->m_phdr;
    for(int i = 0; i < this->m_ehdr->e_phnum; i += 1) {
        if(phdr[i].p_type == type) {
            target = phdr + i;
            break;
        }
    }
    return target;
}

Elf64_Shdr* elf_image64::find_section_by_name(const char *sname) {
    Elf64_Shdr* target = NULL;
    Elf64_Shdr* shdr = this->m_shdr;
    if (!m_str_tab) {
        return NULL;
    }
    for(int i = 0; i < this->m_ehdr->e_shnum; i += 1) {
        const char *name = m_str_tab->get_string((int)shdr[i].sh_name);//(const char *)(shdr[i].sh_name + this->m_shstr_ptr);
        if(name != NULL && !strncmp(name, sname, strlen(sname))) {
            target = (Elf64_Shdr*)(shdr + i);
            break;
        }
    }
    return target;
}