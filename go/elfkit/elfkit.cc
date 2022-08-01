

#include <unistd.h>
#include <stdint.h>

#include <elfkit/elf.h>
#include <elfkit/elf_reader.h>
#include <elfkit/elf_image.h>

#include "elfkit.h"

extern "C" 
ElfReader cgo_elf_reader_create(const char * filename) {
    elf_reader* reader = new elf_reader();
    if (!reader->open(filename)) {
        delete reader;
        reader = NULL;
    }
    return (ElfReader)reader;
}

extern "C"
void cgo_elf_reader_close(ElfReader r) {
    if (r != NULL) {
        elf_reader* reader = (elf_reader*)r;
        delete reader;
    }
    return;
}

extern "C"
ElfImage cgo_elf_reader_load_image(ElfReader r) {
    elf_image* img = NULL;
    if (r != NULL) {
        elf_reader* reader = (elf_reader*)r;
        img = reader->load_image();
    }
    return (ElfImage)img;
}

extern "C"
const char * cgo_elf_image_get_soname(ElfImage img) {
    const char* soname = NULL;
    if (img != NULL) {
        elf_image* image = (elf_image*)img;
        soname = image->get_soname();
    }
    return soname;
}

extern "C"
const char * cgo_elf_image_get_sopath(ElfImage img) {
    const char* sopath = NULL;
    if (img != NULL) {
        elf_image* image = (elf_image*)img;
        sopath = image->get_sopath();
    }
    return sopath;
}

extern "C"
const size_t cgo_elf_image_get_file_size(ElfImage img) {
    size_t file_size = 0;
    if (img != NULL) {
        elf_image* image = (elf_image*)img;
        file_size = image->get_file_size();
    }
    return file_size;
}

extern "C"
const uint8_t cgo_elf_image_get_elf_class(ElfImage img) {
    uint8_t elf_class = ELFCLASSNONE;
    if (img != NULL) {
        elf_image* image = (elf_image*)img;
        elf_class = image->get_elf_class();
    }
    return elf_class;
}

extern "C"
bool cgo_elf_image_get_segment_by_index(ElfImage img, const int index, ElfSegment* segment) {
    if (img != NULL) {
        elf_image* image = (elf_image*)img;
        return image->get_elf_segment_by_index(index, segment);
    }
    return false;
}
extern "C"
bool cgo_elf_image_get_section_by_index(ElfImage img, const int index, ElfSection* section) {
        if (img != NULL) {
        elf_image* image = (elf_image*)img;
        return image->get_elf_section_by_index(index, section);
    }
    return false;
}

extern "C"
bool cgo_elf_image_get_segment_by_type(ElfImage img, const int type, ElfSegment* segment) {
    if (img != NULL) {
        elf_image* image = (elf_image*)img;
        return image->get_elf_segment_by_type(type, segment);
    }
    return false;
}
extern "C"
bool cgo_elf_image_get_section_by_type(ElfImage img, const int type, ElfSection* section) {
    if (img != NULL) {
        elf_image* image = (elf_image*)img;
        return image->get_elf_section_by_type(type, section);
    }
    return false;
}

extern "C"
size_t cgo_elf_image_get_needed_list(ElfImage img, const char** plist) {
    if (img != NULL) {
        elf_image* image = (elf_image*)img;
        string_list needed_list = image->get_needed_list();
        size_t s = needed_list.size();
        if (plist != NULL) {
            for(int i = 0; i < s; i++) {
                plist[i] = needed_list.get(i);
            }
        }
        return s;
    }
    return 0;
}

extern "C"
bool cgo_elf_image_get_symbol_by_name(ElfImage img, const char* name, ElfSymbol* symbol) {
    if (img != NULL) {
        elf_image* image = (elf_image*)img;
        return image->get_symbol_by_name(name, symbol);
    }
    return false;
}

extern "C"
bool cgo_elf_image_get_symbol_by_addr(ElfImage img, const ElfAddr addr, ElfSymbol* symbol) {
    if (img != NULL) {
        elf_image* image = (elf_image*)img;
        return image->get_symbol_by_addr(addr, symbol);
    }
    return false;
}