#include <unistd.h>
#include <stdio.h>

#include "elf_common.h"
#include "elf_reader.h"
#include "elf_image.h"
#include "elf_image32.h"
#include "elf_image64.h"
#include "elf_symbol_tab.h"
#include "elf_reloc_tab.h"

#include "elf_hash_tab.h"
void usage(void) {
    fprintf(stderr, "elfkit sofile");
}

int main(const int argc, const char * args[]) {

    if (argc < 2) {
        usage();
        return -1;
    }
    const char * sopath = args[1];
    printf("so path: %s\n", sopath);

    elf_reader reader;
    if (!reader.open(sopath)) {
        fprintf(stderr, "elf_reader open fail!\n");
        return -1;
    }

    elf_image * image = reader.load_image();
    string_list & needed_list = image->get_needed_list();
    fprintf(stderr, "needed list size: %ld\n", needed_list.size());
    for (int i = 0; i < needed_list.size(); i++) {
        printf("needed: %s\n", needed_list.get(i));
    }
    hash_tab * hashtab = NULL;
    if (image->is_use_gnu_hash()) {
        hashtab = image->get_gnu_hash_tab();
    } else {
        hashtab = image->get_elf_hash_tab();
    }
    elf_symbol_tab * symtab = image->get_symbol_tab();
    if (hashtab && symtab) {
        printf("symbol nums: %d\n", hashtab->get_symbol_nums());
        for(int i = 0; i < hashtab->get_symbol_nums(); i++) {
            elf_symbol sym;
            if (symtab->get_symbol(i, &sym) && sym.sym_name != NULL) {
                printf("%d, sym name:  %s\n", i, sym.sym_name);
            }
        }
    }
    elf_symbol symbol;
    if (image->get_symbol_by_name("printf", &symbol)) {
        printf("find printf, %p, %p\n", symbol.st_value, symbol.st_size);
    }
    image->get_gnu_hash_tab()->dump_hash_table();

    elf_reloc_tab* plt_tab = image->get_plt_tab();
    elf_reloc_tab* rel_tab = image->get_rel_tab();
    printf("plt tab >>>>>\n");
    if (plt_tab) {
        for(int i = 0; i < plt_tab->size(); i++) {
            elf_reloc reloc;
            if (plt_tab->get_reloc(i, &reloc)) {
                printf("  [%d] %p, %llx\n", i, reloc.r_offset, reloc.r_info);
            }
        }
    }
    printf("rel tab >>>>>\n");
    if (rel_tab) {
        for(int i = 0; i < rel_tab->size(); i++) {
            elf_reloc reloc;
            if (rel_tab->get_reloc(i, &reloc)) {
                printf("  [%d] %p, %llx\n", i, reloc.r_offset, reloc.r_info);
            }
        }
    }
    //sleep(100);
    delete image;

    return 0;
}