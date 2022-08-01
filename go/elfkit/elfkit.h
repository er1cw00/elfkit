#include <unistd.h>

#ifdef __cplusplus
extern "C"  {
#endif

#include <elfkit/elf.h>
#include <elfkit/elf_segment.h>
#include <elfkit/elf_section.h>
    
typedef const char* CChar;
typedef void* ElfReader;
typedef void* ElfImage;
typedef elf_section ElfSection;
typedef elf_segment ElfSegment;

ElfReader cgo_elf_reader_create(const char * filename);
void cgo_elf_reader_close(ElfReader reader);
ElfImage cgo_elf_reader_load_image(ElfReader reader);


const char * cgo_elf_image_get_soname(ElfImage img);
const char * cgo_elf_image_get_sopath(ElfImage img);
const size_t cgo_elf_image_get_file_size(ElfImage img);
const uint8_t cgo_elf_image_get_elf_class(ElfImage img); 
bool cgo_elf_image_get_segment_by_index(ElfImage img, const int index, ElfSegment* segment);
bool cgo_elf_image_get_section_by_index(ElfImage img, const int index, ElfSection* section);
bool cgo_elf_image_get_segment_by_type(ElfImage img, const int type, ElfSegment* segment);
bool cgo_elf_image_get_section_by_type(ElfImage img, const int type, ElfSection* section);
size_t cgo_elf_image_get_needed_list(ElfImage img, const char** plist);

#ifdef __cplusplus
};
#endif