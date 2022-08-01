package elfkit

import (
//	"fmt"
//	"runtime"
//	"sync"
	"unsafe"
)

/*
#cgo CFLAGS: -O3 -Wall -Werror -I../../src 
#cgo LDFLAGS: -L../../build -lelfkit-dev -Wl,-rpath,${SRCDIR}/../../
#cgo linux LDFLAGS: -L../../ -lelfkit-dev -lrt -Wl,-rpath,${SRCDIR}/../../
#include "elfkit.h"
*/
import "C"



type ElfReader struct {
	handle C.ElfReader;
}

type ElfImage struct {
	handle C.ElfImage;
}

func NewElfReader(filePath string) *ElfReader {
	r := &ElfReader{}
	r.handle = C.cgo_elf_reader_create(C.CString(filePath));
	return r;
}

func (r *ElfReader) Close() {
	if r != nil && r.handle != nil {
		C.cgo_elf_reader_close(r.handle)
	}
	r.handle = nil;
}

func (r *ElfReader) IsOpen() bool {
	return r != nil && r.handle != nil;
}

func (r *ElfReader) Load() *ElfImage {
	if r != nil && r.handle == nil {
		return nil;
	}
	image := &ElfImage{};
	image.handle = C.cgo_elf_reader_load_image(r.handle);
	return image
}

func (image *ElfImage) GetSoName() string {
	if image != nil && image.handle != nil {
		soname := C.GoString(C.cgo_elf_image_get_soname(image.handle))
		return soname
	}
	return ""
}

func (image *ElfImage) GetSoPath() string {
	if image != nil && image.handle != nil {
		sopath := C.GoString(C.cgo_elf_image_get_sopath(image.handle))
		return sopath
	}
	return ""
}

func (image *ElfImage) GetSectionByIndex(index int) *ElfSection {
	if image == nil || image.handle == nil {
		return nil
	}
	var sec C.ElfSection;
	if C.cgo_elf_image_get_section_by_index(image.handle, C.int(index), &sec) {
		section := NewElfSection(&sec);
		return section;
	}
	return nil;
}

func (image *ElfImage) GetSectionByType(t uint32) *ElfSection {
	if image == nil || image.handle == nil {
		return nil
	}
	var sec C.ElfSection;
	if C.cgo_elf_image_get_section_by_type(image.handle, C.int(t), &sec) {
		section := NewElfSection(&sec);
		return section;
	}
	return nil;
}

func (image *ElfImage) GetSegmentByIndex(index int) *ElfSegment {
	if image == nil || image.handle == nil {
		return nil
	}
	var seg C.ElfSegment;
	if C.cgo_elf_image_get_segment_by_index(image.handle, C.int(index), &seg) {
		segment := NewElfSegment(&seg);
		return segment;
	}
	return nil;
}

func (image *ElfImage) GetSegmentByType(t uint32) *ElfSegment {
	if image == nil || image.handle == nil {
		return nil
	}
	var seg C.ElfSegment;
	if C.cgo_elf_image_get_segment_by_type(image.handle, C.int(t), &seg) {
		segment := NewElfSegment(&seg);
		return segment;
	}
	return nil;
}

func (image *ElfImage) GetNeededList() []string {
	if image == nil || image.handle == nil {
		return nil
	}
	nums := int(C.cgo_elf_image_get_needed_list(image.handle, nil));
	if nums > 0 {
		list := make([]string, 0)
		array := make([]*C.char, nums, nums);
		_ = C.cgo_elf_image_get_needed_list(image.handle, (**C.char)(unsafe.Pointer(&array[0])))
		for i := 0; i < nums; i++ {
			p := array[i];
			list = append(list, C.GoString(p))
		}
		return list
	}
	return nil;
}

func (image *ElfImage) GetSymbolByName(name string) *ElfSymbol {
	if image == nil || image.handle == nil {
		return nil
	}
	var sym C.ElfSymbol
	if C.cgo_elf_image_get_symbol_by_name(image.handle, C.CString(name), &sym) {
		symbol := NewElfSymbol(&sym)
		return symbol
	}
	return nil
}

func (image *ElfImage) GetSymbolByAddr(addr ElfAddr) *ElfSymbol {
	if image == nil || image.handle == nil {
		return nil
	}
	var sym C.ElfSymbol
	if C.cgo_elf_image_get_symbol_by_addr(image.handle, C.ElfAddr(addr), &sym) {
		symbol := NewElfSymbol(&sym)
		return symbol
	}
	return nil
}