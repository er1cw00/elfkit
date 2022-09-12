#include <unistd.h>
#include <stdio.h>

#include <common/elf_common.h>
#include <reader/elf_reader.h>
#include <image/elf_image.h>
#include <model/elf_type.h>
#include <model/elf_reloc_tab.h>

void usage() {
    fprintf(stderr, "./elfkit [sofile]");
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
    
    elf_image* image = reader.load();
    if (!image) {
        fprintf(stderr, "elf_reader load image fail!");
        return -1;
    }

    elf_reloc_tab* plt_tab = image->get_plt_tab();
    elf_reloc_tab* rel_tab = image->get_rel_tab();
    printf("plt tab >>>>>\n");
    if (plt_tab) {
        elf_reloc_list_t list;
        plt_tab->get_list(list);
        for (int i = 0; i < list.size(); i++) {
            elf_reloc & reloc = list[i];
            fprintf(stderr, "reloc(%d), offset(%p), addend(%p), info(%16llx)\n", 
                            i,
                            (void*)reloc.r_offset,
                            (void*)reloc.r_addend,
                            (uint64_t)reloc.r_info);
        }
    }
    
    printf("rel tab >>>>>\n");
    if (rel_tab) {
        elf_reloc_list_t list;
        rel_tab->get_list(list);
        for (int i = 0; i < list.size(); i++) {
            elf_reloc & reloc = list[i];
            fprintf(stderr, "reloc(%d), offset(%p), addend(%p), info(%16llx)\n",
                            i,
                            (void*)reloc.r_offset,
                            (void*)reloc.r_addend,
                            (uint64_t)reloc.r_info);
        }
    }
    
    return 0;
}