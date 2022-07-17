#include <unistd.h>

#ifdef __cplusplus
extern "C"  {
#endif

#include <elfkit/elf.h>

typedef void* ElfReader;
typedef void* ElfImage;

ElfReader cgo_elf_reader_create(const char * filename);
void cgo_elf_reader_close(ElfReader reader);
ElfImage cgo_elf_reader_load_image(ElfReader reader);


const char * cgo_elf_image_get_soname(ElfImage img);
const char * cgo_elf_image_get_sopath(ElfImage img);
const size_t cgo_elf_image_get_file_size(ElfImage img);
const uint8_t cgo_elf_image_get_elf_class(ElfImage img); 


#ifdef __cplusplus
};
#endif