#include <unistd.h>
#include <stdio.h>

#include <common/elf_common.h>
#include <reader/elf_reader.h>
#include <image/elf_image.h>
#include <model/elf_type.h>
#include <model/elf_reloc_tab.h>
#include <model/elf_symbol_tab.h>

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

    Elf64_Phdr* phdr = (Elf64_Phdr*)reader.get_phdr_base();
    size_t phnum = reader.get_phdr_num();
    for (int i = 0; i < phnum; i++) {
        elf_segment segment;
        elf_segment_reset_with_phdr64(&segment, phdr+i);
        printf("i:%d, type:%d, addr:%llx\n", i, segment.p_type, segment.p_offset);
    }

    elf_hash_tab* gnu_hash_tab = image->get_gnu_hash_tab();
    elf_hash_tab* sysv_hash_tab = image->get_sysv_hash_tab();
    elf_symbol_tab* symtab = image->get_sym_tab();

    elf_reloc_tab* plt_tab = image->get_plt_tab();
    elf_reloc_tab* rel_tab = image->get_rel_tab();
    printf("plt tab >>>>>\n");
    if (plt_tab) {
        elf_reloc_list_t& list = plt_tab->get_list();
        for (int i = 0; i < list.size(); i++) {
            elf_reloc & reloc = list[i];
            int sym_idx = elf_reloc_get_symbol_index(&reloc);
            int sym_type = elf_reloc_get_symbol_type(&reloc);
                 const char* sym_name = "<<not found>>";
            elf_symbol sym;
            if (symtab->get_symbol(sym_idx, &sym)) {
                sym_name = sym.sym_name;
            }
            fprintf(stdout, "reloc(%d), offset(%p), addend(%p), sym(%d), type(%x), name(%s)\n", 
                            i,
                            (void*)reloc.r_offset,
                            (void*)reloc.r_addend,
                            sym_idx,
                            sym_type,
                            sym_name);
        }
    }
    
    printf("rel tab >>>>>\n");
    if (rel_tab) {
        elf_reloc_list_t list = rel_tab->get_list();
        for (int i = 0; i < list.size(); i++) {
            elf_reloc & reloc = list[i];
            int sym_idx = elf_reloc_get_symbol_index(&reloc);
            int sym_type = elf_reloc_get_symbol_type(&reloc);
            const char* sym_name = "<<not found>>";
            elf_symbol sym;
            if (symtab->get_symbol(sym_idx, &sym)) {
                sym_name = sym.sym_name;
            }
            fprintf(stdout, "reloc(%d), offset(%p), addend(%p), sym(%d), type(%x), name(%s)\n",
                            i,
                            (void*)reloc.r_offset,
                            (void*)reloc.r_addend,
                            sym_idx,
                            sym_type,
                            sym_name);
        }
    }
    image->unload();
    return 0;
}