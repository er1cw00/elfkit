#include <unistd.h>
#include <stdio.h>

#include "elf_common.h"
#include "elf_image.h"

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
    elf_image * image = elf_image::create(sopath);
    if (image) {
        image->load();
    }

    return 0;
}