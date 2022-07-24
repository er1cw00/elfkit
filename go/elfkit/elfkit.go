package elfkit

//import (
//	"runtime"
//	"sync"
//	"unsafe"
//)

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

}

func (image *ElfImage) GetSectionByType(type int32) *ElfSection {
	
}

func (image *ElfImage) GetSegmentByIndex(index int) *ElfSegment {
	if image != nil && image.handle != nil  {
		C C.cgo_elf_image_get_segment_by_index(index);
	}
}

func (image *ElfImage) GetSegmentByType(type int32) *ElfSegment {
	
}