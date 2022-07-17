#include <unistd.h>
#include <stdio.h>

#include "elf_common.h"
#include "elf_reader.h"
#include "elf_image.h"
#include "elf_image32.h"
#include "elf_image64.h"
#include "elf_symbol_tab.h"

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
    fprintf(stderr, "01\n");
    elf_needed_list_t needed_list = image->get_needed_list();
    fprintf(stderr, "2\n");
    fprintf(stderr, "size: %d\n", needed_list.size());
    for (elf_needed_list_t::iterator itor = needed_list.begin(); itor != needed_list.end(); itor++) {
        printf("needed: %s\n", itor->c_str());
    }
    elf_symbol_tab * sym_tab = image->get_symbol_tab();
    // int i = 0;
    // while(sym_tab) {
    //     elf_symbol * sym = sym_tab->get_symbol(i);
    //     if (sym != NULL) {
    //         printf("%d, sym name:   %s\n", i, sym->get_sym_name());
    //     }
    //     i +=1;
    // }
    sleep(100);
    delete image;

    return 0;
}