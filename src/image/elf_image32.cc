#include <vector>
#include <string>

#include <common/elf.h>
#include <common/elf_log.h>
#include <common/elf_common.h>
#include <model/elf_type.h>
#include <model/elf_hash_tab.h>
#include <model/elf_sysv_hash_tab.h>
#include <model/elf_gnu_hash_tab.h>
#include <model/elf_reloc_tab.h>
#include <model/elf_symbol_tab.h>
#include <model/elf_string_tab.h>
#include <model/elf_func_array.h>
#include <image/elf_image32.h>

elf_image32::elf_image32(elf_reader* reader) : elf_image(reader)  {
    m_arm_exidx_offset = NULL;
    m_arm_exidx_count = 0;
}

elf_image32::~elf_image32() {
}

bool elf_image32::load() {
   this->m_ehdr = (Elf32_Ehdr*)this->get_load_base();
    if (this->m_ehdr->e_type != ET_EXEC && this->m_ehdr->e_type != ET_DYN) {
        return false;
    }
    if (this->get_machine_type() == EM_ARM) {
        _load_arm_exidx();
    }
    Elf32_Phdr * dyn_phdr = _find_segment_by_type(PT_DYNAMIC);
    if (dyn_phdr == NULL) {
        return  false;
    }
    this->m_dynamic             = (void*)(this->get_load_bias() + dyn_phdr->p_vaddr);
    this->m_dynamic_size        = (size_t)(dyn_phdr->p_memsz/sizeof(Elf32_Dyn));

    Elf32_Sym* symtab           = NULL;
    size_t sym_size             = 0;

    const char* strtab          = NULL;
    size_t strtab_size          = 0;

    addr_t plt_offset           = NULL;
    size_t plt_size             = 0;
    addr_t rel_offset           = NULL;
    size_t rel_size             = 0;
    size_t rel_count            = 0;
    size_t rel_entry_size       = 0;

    addr_t relr_offset          = NULL;
    size_t relr_size            = 0;

    addr_t init_array           = 0;
    addr_t fini_array          = 0;
    addr_t preinit_array        = 0;
    size_t init_array_count     = 0;
    size_t fini_array_count    = 0;
    size_t preinit_array_count  = 0;

    std::vector<int> needed_list;

    for (Elf32_Dyn* d = (Elf32_Dyn*)this->m_dynamic; d->d_tag != DT_NULL; ++d) {
        //log_warn("unsupport dyn tag: %s", elf_dynamic_tag_name(d->d_tag));
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
                this->m_sysv_hash_tab = new elf_sysv_hash_tab(nbucket, nchain, bucket, chain);
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
                uint32_t* gnu_bucket       = (uint32_t *)(gnu_bloom_filter + gnu_maskwords * 4);
                uint32_t* gnu_chain        = (uint32_t *)(gnu_bucket + gnu_nbucket - gnu_symndx);
                if (!powerof2(gnu_maskwords)) {
                    log_error("invalid maskwords for gnu_hash = 0x%x, in \"%s\" expecting power to two.",
                            gnu_maskwords, get_soname());
                    return false;
                }
                gnu_maskwords -= 1;
                this->m_gnu_hash_tab = new elf_gnu_hash_tab(get_elf_class(),
                                                            gnu_nbucket, 
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
                symtab = (Elf32_Sym*)(this->get_load_bias() + d->d_un.d_ptr);
                break;
            case DT_SYMENT:
                if (d->d_un.d_ptr != sizeof(Elf32_Sym)) {
                    log_error("invalid DT_SYMENT: %zd in \"%s\".", (size_t)d->d_un.d_ptr, get_soname());
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
                this->m_fini_func = (addr_t)(this->get_load_bias() + d->d_un.d_ptr);
                break;
            case DT_INIT_ARRAY:
                init_array = (addr_t)(this->get_load_bias() + d->d_un.d_ptr);
                break;
            case DT_INIT_ARRAYSZ:
                init_array_count = (size_t)(d->d_un.d_val) / sizeof(Elf32_Addr);
                break;
            case DT_FINI_ARRAY:
                fini_array = (addr_t)(this->get_load_bias() + d->d_un.d_ptr);
                break;            
            case DT_FINI_ARRAYSZ:
                fini_array_count = (size_t)(d->d_un.d_val) / sizeof(Elf32_Addr);
                break;
            case DT_PREINIT_ARRAY:
                preinit_array = (addr_t)(this->get_load_bias() + d->d_un.d_ptr);
                break;
            case DT_PREINIT_ARRAYSZ:
                preinit_array_count = (size_t)(d->d_un.d_val) / sizeof(Elf32_Addr);
                break;
            case DT_PLTREL:
                this->m_is_use_rela = (d->d_un.d_val == DT_RELA);
                break;
            case DT_JMPREL:
                plt_offset = (addr_t)(this->get_load_bias() + d->d_un.d_ptr);
                break;
            case DT_PLTRELSZ:
                plt_size = (size_t)d->d_un.d_val;
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
            case DT_RELCOUNT:
            case DT_RELACOUNT:
                //rel_count = (size_t)d->d_un.d_val;
                break;
            case DT_RELRSZ:
            case DT_ANDROID_RELRSZ:
                relr_size = (size_t)d->d_un.d_val;
                break;
            case DT_RELR:
            case DT_ANDROID_RELR:
                relr_offset = (addr_t)(this->get_load_bias() + d->d_un.d_ptr);
                break;
            case DT_PLTGOT:
            case DT_DEBUG:
            case DT_RUNPATH:
            default:
                //log_warn("unsupport dyn tag: %s", elf_dynamic_tag_name(d->d_tag));
                break;
        }
    }
    _create_str_tab(strtab, strtab_size);

    _create_func_array(init_array, init_array_count,
                       fini_array, fini_array_count, 
                       preinit_array, preinit_array_count);
    _create_needed_list(needed_list);
    _create_symbol_tab(symtab);

    assert(rel_entry_size == sizeof(Elf32_Rel) || rel_entry_size == sizeof(Elf32_Rela));
    _create_reloc_tab(relr_offset, relr_size,
                      rel_offset, rel_size,
                      plt_offset, plt_size,
                      rel_entry_size);
    return true;
}
void elf_image32::unload() {
    return;
}
addr_t elf_image32::get_arm_exidx_offset() {
    if (this->get_machine_type() != EM_ARM) {
        return NULL;
    }
    return this->m_arm_exidx_offset;
}
size_t elf_image32::get_arm_exidx_count() {
    if (this->get_machine_type() != EM_ARM) {
        return 0;
    }
    return this->m_arm_exidx_count;
}
void elf_image32::_create_symbol_tab(Elf32_Sym* symtab) {
    if (symtab) {
        this->m_sym_tab = new elf_symbol_tab(symtab, 0, this->m_str_tab);
    }
}
void elf_image32::_load_arm_exidx() {
    Elf32_Phdr* phdr = _find_segment_by_type(PT_ARM_EXIDX);
    if (phdr == NULL) {
        log_warn("there is not arm.exidx segment in so (%s)!", this->get_soname());
        this->m_arm_exidx_offset = NULL;
        this->m_arm_exidx_count = 0;
        return;
    } 
    this->m_arm_exidx_offset = (addr_t)phdr->p_vaddr;
    this->m_arm_exidx_count = (size_t)phdr->p_memsz / 8;
    return;
}

Elf32_Phdr* elf_image32::_find_segment_by_type(const uint32_t type) {
    Elf32_Phdr* target = NULL;
    Elf32_Phdr* phdr = (Elf32_Phdr*)this->m_reader->get_phdr_base();
    size_t phnum = this->m_reader->get_phdr_num();
    for(int i = 0; i < phnum; i += 1) {
        if(phdr[i].p_type == type) {
            target = phdr + i;
            break;
        }
    }
    return target;
}
