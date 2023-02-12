#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <limits.h>

#include <common/elf_common.h>
#include <file/elf_reader.h>
#include <image/elf_image.h>
#include <model/elf_type.h>
#include <model/elf_hash_tab.h>
#include <model/elf_reloc_tab.h>
#include <model/elf_symbol_tab.h>
#include <model/elf_func_array.h>

void usage() {

    fprintf(stderr, "./elfkit [sofile] options\n");
    fprintf(stderr, "    --help print this message\n");
    fprintf(stderr, "    --file input elf file path\n");
    fprintf(stderr, "    --init print init func\n");
}

struct option long_opts[] = {
    {"help",     no_argument,       0, 'h'},
    {"file",     required_argument, 0, 'f'},
    {"section",  no_argument,       0, 'S'},
    {"program",  no_argument,       0, 'l'},
    {"dynamic",  no_argument,       0, 'd'},
    {"init",     no_argument,       0, 'i'},
    {"needed",   no_argument,       0, 'n'},
    {"hash",     no_argument,       0, 'a'},
    {"sym",      no_argument,       0, 's'},
    {"reloc",    no_argument,       0, 'r'},
    {NULL, 0, 0, 0}
};

bool __show_help          = false;
bool __show_section       = false;
bool __show_program       = false;
bool __show_dynamic       = false;
bool __show_needed_lib    = false;
bool __show_init_func     = false;
bool __show_hash_tab      = false;
bool __show_sym_tab       = false;
bool __show_reloc_tab     = false;

char __sopath[PATH_MAX];

bool parse_opts(const int argc, char *const * args) {
    bool result = false;
    while(1) {
        int index = -1;
        int c = getopt_long_only(argc, args, "hf:Sldinasr", long_opts, &index);
        if (c < 0) {
            break;
        } 
        switch (c) {
            case 'f': strncpy(__sopath, optarg, sizeof(__sopath));result = true; break;
            case 'S': __show_section    = true; break;
            case 'l': __show_program    = true; break;
            case 'd': __show_dynamic    = true; break;
            case 'n': __show_needed_lib = true; break;
            case 'i': __show_init_func  = true; break;
            case 'a': __show_hash_tab   = true; break;
            case 's': __show_sym_tab    = true; break;
            case 'r': __show_reloc_tab  = true; break;
            case 'h': 
            default: __show_help = true; break;
        }
    }
    return result;
}
void dump_func_array(const char *tag, elf_func_array* array) {
    if (array == NULL) {
        printf(".%s is empty!\n", tag);
        return;
    } 
    uint64_t* addr = (uint64_t*)array->get_func_array();
    size_t count = array->get_func_count();
    if (addr == NULL || count == 0) {
        printf(".%s pointer is %p, count is %ld\n", tag, addr, count);
        return;
    }
    printf(".%s has %ld func: [\n", tag, count);
    for (int i = 0; i < count; i++) {
        if (i < count - 1) {
            printf("0x%p, ", (uint64_t*)addr[i]);
        } else {
            printf("0x%p", (uint64_t*)addr[i]);
        }
    }
    printf("]\n");
}
void show_section(elf_image* image) {

}



void show_program(elf_reader* reader, elf_image* image) {
    
    if (reader->get_elf_class() == ELFCLASS32) {
        printf("Index    Type        Offset    VirAddr    PhyAddr     Filesz    Memsz     Flag    Align\n");
        Elf32_Phdr* phdr = (Elf32_Phdr*)reader->get_phdr_base();
        size_t phnum = reader->get_phdr_num();
        for (int i = 0; i < phnum; i++) {
            printf("%02d  %s   %08x   %08x   %08x   %06x   %06x    %s   %4x\n", 
                            i, 
                            elf_phdr_type_name(phdr[i].p_type), 
                            phdr[i].p_offset,
                            phdr[i].p_vaddr,
                            phdr[i].p_paddr,
                            phdr[i].p_filesz,
                            phdr[i].p_memsz,
                            elf_protection_flag_name(phdr[i].p_flags),
                            phdr[i].p_align);
        }
    } else if (reader->get_elf_class() == ELFCLASS64) {
        printf("Index    Type          Offset             VirAddr         PhyAddr        Filesz       Memsz    Flag    Align\n");
        Elf64_Phdr* phdr = (Elf64_Phdr*)reader->get_phdr_base();
        size_t phnum = reader->get_phdr_num();
        for (int i = 0; i < phnum; i++) {
            printf("  %02d     %s       %012x       %012x   %012x   %06x   %06x    %s   %4x\n", 
                            i, 
                            elf_phdr_type_name(phdr[i].p_type), 
                            phdr[i].p_offset,
                            phdr[i].p_vaddr,
                            phdr[i].p_paddr,
                            phdr[i].p_filesz,
                            phdr[i].p_memsz,
                            elf_protection_flag_name(phdr[i].p_flags),
                            phdr[i].p_align);
        }
    } 
}
void show_init_func(elf_image* image) {
    printf("init_func addr: %p\n",      (void*)image->get_init_func());
    printf("finit_func addr: %p\n",     (void*)image->get_finit_func());
    dump_func_array("init_array:",      image->get_init_array());
    dump_func_array("finit_array:",     image->get_finit_array());
    dump_func_array("preinit_array:",   image->get_preinit_array());
}

void show_sym_tab(elf_image* image) {

}
void show_reloc_tab(elf_image* image) {

}

int main(const int argc, char *const * args) {
    if (!parse_opts(argc, args) || strlen(__sopath) == 0) {
        usage();
        return -1;
    }
    if ( __show_help) {
        usage();
        return 0;
    }
    char *sopath = __sopath;
//    sopath=args[1];
    printf("so path: (%s)\n", sopath);
    elf_reader* reader = new elf_reader();
    if (!reader->open(sopath)) {
        fprintf(stderr, "elf_reader open fail!\n");
        return -1;
    }
    elf_image* image = reader->load();
    if (!image) {
        fprintf(stderr, "elf_reader load image fail!");
        return -1;
    }
    if (__show_section) {
        show_section(image);
    }
    if (__show_program) {
        show_program(reader, image);
    }
    if (__show_init_func) {
        show_init_func(image);
    } 
    if (__show_sym_tab) {
        show_sym_tab(image);      
    } 
    if (__show_reloc_tab) {
        show_reloc_tab(image);
    }
    return 0;
}



    
   



// #if 0
//     Elf64_Phdr* phdr = (Elf64_Phdr*)reader->get_phdr_base();
//     size_t phnum = reader->get_phdr_num();
//     for (int i = 0; i < phnum; i++) {
//         elf_segment segment;
//         elf_segment_reset_with_phdr64(&segment, phdr+i);
//         printf("i:%d, type:%d, addr:%llx\n", i, segment.p_type, segment.p_offset);
//     }
// #endif
//     elf_hash_tab* gnu_hash_tab = image->get_gnu_hash_tab();
//     elf_hash_tab* sysv_hash_tab = image->get_sysv_hash_tab();
//     elf_symbol_tab* symtab = image->get_sym_tab();

//     size_t s1 = gnu_hash_tab != NULL ? gnu_hash_tab->get_symbol_nums() : 0;
//     size_t s2 = sysv_hash_tab != NULL ? sysv_hash_tab->get_symbol_nums() : 0;

//     elf_symbol symbol;
//     bool r = image->get_symbol_by_name("__res_hnok", &symbol);//gnu_hash_tab->find_symbol_by_name(symtab, "__res_hnok", &symbol);
//     fprintf(stderr, "r: %d\n", r);
//     return 0;
//     //if (symtab) {
//         size_t total = s1+s2;//symtab->size();
//         fprintf(stderr, "symtab size: %lu+%lu=%lu\n", s1, s2, total);

//         for (int symidx = 0; symidx < total; symidx++) {
//             const char* sym_name = "<<not found>>";
//             elf_symbol sym;
//             if (symtab->get_symbol(symidx, &sym)) {
//                 uint32_t bind = ELF_ST_BIND(sym.st_info);
//                 uint32_t type = ELF_ST_TYPE(sym.st_info);
//                 fprintf(stderr, "sym[%d]: name_idx:%d,info:0x%x,other:%d,shndx:0x%x,value:0x%lx,size:%ld,bind:%x,type:%x,name:%s\n", 
//                                   symidx, 
//                                   sym.st_name,
//                                   sym.st_info,
//                                   sym.st_other,
//                                   sym.st_shndx,
//                                   sym.st_value,
//                                   sym.st_size,
//                                   bind, 
//                                   type,
//                                   sym.sym_name);
//             }
//             //fprintf(stderr, "sym[%d]: %s\n", symidx, sym_name);
//         }
//     //}
//     elf_reloc_tab* plt_tab = image->get_plt_tab();
//     elf_reloc_tab* rel_tab = image->get_rel_tab();
// #if 0
//     printf("plt tab >>>>>\n");
//     if (plt_tab) {
//         elf_reloc_list_t& list = plt_tab->get_list();
//         for (int i = 0; i < list.size(); i++) {
//             elf_reloc & reloc = list[i];
//             int sym_idx = elf_reloc_get_symbol_index(&reloc);
//             int sym_type = elf_reloc_get_symbol_type(&reloc);
//                  const char* sym_name = "<<not found>>";
//             elf_symbol sym;
//             if (symtab->get_symbol(sym_idx, &sym)) {
//                 sym_name = sym.sym_name;
//             }
//             fprintf(stdout, "reloc(%d), offset(%p), addend(%p), sym(%d), type(%x), name(%s)\n", 
//                             i,
//                             (void*)reloc.r_offset,
//                             (void*)reloc.r_addend,
//                             sym_idx,
//                             sym_type,
//                             sym_name);
//         }
//     }
// #endif

// #if 0

//     printf("rel tab >>>>>\n");
//     if (rel_tab) {
//         elf_reloc_list_t list = rel_tab->get_list();
//         for (int i = 0; i < list.size(); i++) {
//             elf_reloc & reloc = list[i];
//             int sym_idx = elf_reloc_get_symbol_index(&reloc);
//             int sym_type = elf_reloc_get_symbol_type(&reloc);
//             const char* sym_name = "<<not found>>";
//             elf_symbol sym;
//             if (symtab->get_symbol(sym_idx, &sym)) {
//                 sym_name = sym.sym_name;
//             }
//             uint64_t* p = (uint64_t*)(reloc.r_offset + image->get_load_bias());
//             fprintf(stdout, "reloc(%d), offset(%p), addend(%p), sym(%d), type(%x), name(%s), p(%p), value(%012llx)\n",
//                             i,
//                             (void*)reloc.r_offset,
//                             (void*)reloc.r_addend,
//                             sym_idx,
//                             sym_type,
//                             sym_name,
//                             p,
//                             *(p+1));
//         }
//     }
// #endif
//     image->unload();
//     return 0;
// }