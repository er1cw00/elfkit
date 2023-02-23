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

    fprintf(stderr, "./elfkit sofile [-hSldinasre]\n");
    fprintf(stderr, "    --help, -h      print this message\n");
    fprintf(stderr, "    --section, -S   print sections\n");
    fprintf(stderr, "    --program, -l   print segments\n");
    fprintf(stderr, "    --dynamic, -d   print dynamic segment\n");
    fprintf(stderr, "    --init, -i      print init func\n");
    fprintf(stderr, "    --needed, -n    print needed library list\n");
    fprintf(stderr, "    --hash, -a      print hash tab\n");
    fprintf(stderr, "    --sym, -s       print symbol\n");
    fprintf(stderr, "    --reloc, -r     print relocation\n");
    fprintf(stderr, "    --armexidx, -e  print arm.exidx offset and count\n");

}

struct option long_opts[] = {
    {"help",     no_argument,       0, 'h'},
    {"section",  no_argument,       0, 'S'},
    {"program",  no_argument,       0, 'l'},
    {"dynamic",  no_argument,       0, 'd'},
    {"init",     no_argument,       0, 'i'},
    {"needed",   no_argument,       0, 'n'},
    {"hash",     no_argument,       0, 'a'},
    {"sym",      no_argument,       0, 's'},
    {"reloc",    no_argument,       0, 'r'},
    {"armexidx", no_argument,       0, 'e'},
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
bool __show_arm_exidx     = false;
char __sopath[PATH_MAX];

bool parse_opts(const int argc, char *const * args) {
    while(1) {
        int index = -1;
        int c = getopt_long(argc, args, "hSldinasre", long_opts, &index);
        if (c < 0) {
            break;
        } 
        switch (c) {
            case 'f':  break;
            case 'S': __show_section    = true; break;
            case 'l': __show_program    = true; break;
            case 'd': __show_dynamic    = true; break;
            case 'n': __show_needed_lib = true; break;
            case 'i': __show_init_func  = true; break;
            case 'a': __show_hash_tab   = true; break;
            case 's': __show_sym_tab    = true; break;
            case 'r': __show_reloc_tab  = true; break;
            case 'e':__show_arm_exidx   = true; break;
            case 'h': 
            default: __show_help = true; break;
        }
    }
    if (optind < argc) {
        strncpy(__sopath, args[optind], sizeof(__sopath));
        return true;
    }
    return false;
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
    size_t shnum = image->get_section_size();
    if (shnum <= 0) {return;}
    elf_section * shdr = (elf_section*)malloc(sizeof(elf_section) * shnum);
    if (!shdr) {return;}
    image->get_section_list(shdr);
    printf("Index  Name                    Type          Addr          Offset        Size    ES  Align\n");
    for (int i = 0; i < shnum; i++) {
        printf("  %02d  %-22s   %-12s  %012llx  %012llx  %06llx  %02llx  %04llx\n", 
                            i, 
                            shdr[i].name, 
                            elf_shdr_type_name(shdr[i].sh_type),
                            shdr[i].sh_addr,
                            shdr[i].sh_offset,
                            shdr[i].sh_size,
                            shdr[i].sh_entsize,
                            shdr[i].sh_addralign);
    }
    return;
}

void show_program(elf_image* image) {
    size_t phnum = image->get_segment_size();
    if (phnum <= 0) {return;}
    elf_segment * phdr = (elf_segment*)malloc(sizeof(elf_segment) * phnum);
    if (!phdr) {return;}
    image->get_segment_list(phdr);
    printf("Index    Type         Offset         VirAddr        PhyAddr        Filesz   Memsz     Flag  Align\n");
    for (int i = 0; i < phnum; i++) {
            printf("  %02d     %-10s   %012llx   %012llx   %012llx   %06llx   %06llx    %s   %4llx\n", 
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
    return;
}

void show_dynamic(elf_image* image) {
    size_t dyn_num = image->get_dynamic_size();
    if (dyn_num <= 0) {return;}
    elf_dynamic * dyn = (elf_dynamic*)malloc(sizeof(elf_dynamic) * dyn_num);
    if (dyn == NULL) {return;}
    image->get_dynamic_list(dyn);
    printf("Index  Name          Value\n");
    for (int i = 0; i < dyn_num; i++) {
        printf(" %2d  %-14s  0x%016llx\n",
                i,
                elf_dynamic_tag_name(dyn[i].d_tag), 
                uint64_t(dyn[i].d_val));
    }
    return;
}

void show_needed_lib_list(elf_image *image) {
    elf_string_list needed_list = image->get_needed_list();
    printf("Needed library contains : [\n");
    for (int i = 0; i < needed_list.size(); i++) {
        printf("  %s\n", needed_list.get(i));
    }
    printf("]\n");
}
void show_init_func(elf_image* image) {
    printf("init_func addr: %p\n",      (void*)image->get_init_func());
    printf("finit_func addr: %p\n",     (void*)image->get_finit_func());
    dump_func_array("init_array:",      image->get_init_array());
    dump_func_array("finit_array:",     image->get_finit_array());
    dump_func_array("preinit_array:",   image->get_preinit_array());
}

void show_hash_tab(elf_image* image) {
    elf_hash_tab* gnu_hash_tab = image->get_gnu_hash_tab();
    if (gnu_hash_tab) {
        gnu_hash_tab->dump_hash_table();
    }
    elf_hash_tab* sysv_hash_tab = image->get_sysv_hash_tab();
    if (sysv_hash_tab) {
        sysv_hash_tab->dump_hash_table();
    }
}

void show_dynsym_tab(elf_image* image) {
    elf_hash_tab* gnu_hash_tab = image->get_gnu_hash_tab();
    elf_hash_tab* sysv_hash_tab = image->get_sysv_hash_tab();
    elf_symbol_tab* symtab = image->get_sym_tab();
    size_t s1 = gnu_hash_tab != NULL ? gnu_hash_tab->get_symbol_nums() : 0;
    size_t s2 = sysv_hash_tab != NULL ? sysv_hash_tab->get_symbol_nums() : 0;
    if (symtab) {
        size_t total = s1 + s2;
        printf("Symbol Table contains %zd entries:\n", total);
        printf("Index  Value       Size  Type    Bind    Name  \n");
        for (int symidx = 0; symidx < total; symidx++) {
            const char* sym_name = "<<not found>>";
            elf_symbol sym;
            if (symtab->get_symbol(symidx, &sym)) {
                uint32_t bind = ELF_ST_BIND(sym.st_info);
                uint32_t type = ELF_ST_TYPE(sym.st_info);
                if (sym.sym_name) {sym_name = sym.sym_name;}
                printf("%5d  %010llx  %3zd   %-6s  %-6s  %s\n",
                    symidx, 
                    sym.st_value,
                    sym.st_size,
                    elf_sym_bind_name(bind),
                    elf_sym_type_name(type),
                    sym_name);
            }
        }
    }
}

void show_reloc_tab(elf_image* image) {
    elf_reloc_tab* plt_tab = image->get_plt_tab();
    elf_reloc_tab* rel_tab = image->get_rel_tab();
    elf_symbol_tab* symtab = image->get_sym_tab();
    uint16_t em = image->get_machine_type();
    if (plt_tab) {
        elf_reloc_list_t& list = plt_tab->get_list();
        printf("Relocation section \'.%s.plt\' contains %zd entries:\n", 
                        rel_tab->is_use_rela() ? "rela" : "rel",
                        list.size());
        printf("Index   Offset        SymIdx       SymType           Sym.Value     Sym.Name\n");
        for (int i = 0; i < list.size(); i++) {
            elf_reloc & reloc = list[i];
             const char* sym_name = "<<not found>>";
            int sym_idx = elf_reloc_get_symbol_index(&reloc);
            int sym_type = elf_reloc_get_symbol_type(&reloc);
            elf_symbol sym;
            if (symtab->get_symbol(sym_idx, &sym) && sym.sym_name) {sym_name = sym.sym_name;}
            printf("%5d   %012llx  %5d   %-18s     %012llx  %s \n",
                    i,
                    reloc.r_offset,
                    sym_idx,
                    elf_reloc_stype_name(em, sym_type),
                    sym.st_value,
                    sym_name);
        }
    }
    if (rel_tab) {
        elf_reloc_list_t& list = rel_tab->get_list();
        printf("Relocation section \'.%s.dyn\' contains %zd entries:\n", 
                        rel_tab->is_use_rela() ? "rela" : "rel",
                        list.size());
        for (int i = 0; i < list.size(); i++) {
            elf_reloc & reloc = list[i];
             const char* sym_name = "<<not found>>";
            int sym_idx = elf_reloc_get_symbol_index(&reloc);
            int sym_type = elf_reloc_get_symbol_type(&reloc);
            elf_symbol sym;
            if (symtab->get_symbol(sym_idx, &sym) && sym.sym_name) {sym_name = sym.sym_name;}
            printf("%5d   %012llx  %5d   %-18s     %012llx  %s \n",
                    i,
                    reloc.r_offset,
                    sym_idx,
                    elf_reloc_stype_name(em, sym_type),
                    sym.st_value,
                    sym_name);
        }
    }
}
void show_arm_exidx(elf_image* image) {
    printf("arm.exidx offset: 0x%x, count: %zd\n", 
        (uint32_t)image->get_arm_exidx_offset(), 
        image->get_arm_exidx_count());
    return;
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
        show_program(image);
    }
    if (__show_dynamic) {
        show_dynamic(image);
    }
    if (__show_init_func) {
        show_init_func(image);
    } 
    if (__show_needed_lib) {
        show_needed_lib_list(image);
    }
    if (__show_hash_tab) {
        show_hash_tab(image);
    }
    if (__show_sym_tab) {
        show_dynsym_tab(image);      
    } 
    if (__show_reloc_tab) {
        show_reloc_tab(image);
    }
    if (__show_arm_exidx) {
        show_arm_exidx(image);
    }
    return 0;
}
