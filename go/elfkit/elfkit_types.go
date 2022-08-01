package elfkit

import (
	"fmt"
//	"runtime"
//	"sync"
//	"unsafe"
)

/*
#include "elfkit.h"
*/
import "C"

type ElfAddr   uintptr
type ElfOffset uintptr
type ElfSize   uint64

const PT_NULL          uint32 = C.PT_NULL
const PT_LOAD          uint32 = C.PT_LOAD
const PT_DYNAMIC       uint32 = C.PT_DYNAMIC
const PT_INTERP        uint32 = C.PT_INTERP
const PT_NOTE          uint32 = C.PT_NOTE
const PT_SHLIB         uint32 = C.PT_SHLIB
const PT_PHDR          uint32 = C.PT_PHDR
const PT_TLS           uint32 = C.PT_TLS
const PT_LOOS          uint32 = C.PT_LOOS
const PT_HIOS          uint32 = C.PT_HIOS
const PT_LOPROC        uint32 = C.PT_LOPROC
const PT_HIPROC        uint32 = C.PT_HIPROC
const PT_GNU_EH_FRAME  uint32 = C.PT_GNU_EH_FRAME
const PT_GNU_PROPERTY  uint32 = C.PT_GNU_PROPERTY
const PT_GNU_STACK     uint32 = C.PT_GNU_STACK
const PT_ARM_EXIDX     uint32 = C.PT_ARM_EXIDX
const PN_XNUM          uint32 = C.PN_XNUM

type ElfSegment struct {
    Type      uint32;
    Flags     uint32;
    Offset    ElfOffset;
    VirtAddr  ElfAddr;
    PhyAddr   ElfAddr;
    FileSize  ElfSize;
    MemSize   ElfSize;
    Align     uint64;
}

func NewElfSegment(seg *C.ElfSegment) *ElfSegment {
	segment := &ElfSegment {
			Type:      uint32(seg.p_type),
			Flags:     uint32(seg.p_flags),
			Offset:    ElfOffset(seg.p_offset),
			VirtAddr:  ElfAddr(seg.p_vaddr),
			PhyAddr:   ElfAddr(seg.p_paddr),
			FileSize:  ElfSize(seg.p_filesz),
			MemSize:   ElfSize(seg.p_memsz),
			Align:     uint64(seg.p_align),
		}
	return segment;
}

func (segment *ElfSegment) String() string {
	var result string = "";
	result = fmt.Sprintf("%d,  %x,  %x    %x    %x    %d   %d   %d",
					segment.Type,
					segment.Flags,
					segment.Offset,
					segment.VirtAddr,
					segment.PhyAddr,
					segment.FileSize,
					segment.MemSize,
					segment.Align);
	return result;
}


const SHT_NULL           uint32 = C.SHT_NULL
const SHT_PROGBITS       uint32 = C.SHT_PROGBITS
const SHT_SYMTAB         uint32 = C.SHT_SYMTAB
const SHT_STRTAB         uint32 = C.SHT_STRTAB
const SHT_RELA           uint32 = C.SHT_RELA
const SHT_HASH           uint32 = C.SHT_HASH
const SHT_DYNAMIC        uint32 = C.SHT_DYNAMIC
const SHT_NOTE           uint32 = C.SHT_NOTE
const SHT_NOBITS         uint32 = C.SHT_NOBITS
const SHT_REL            uint32 = C.SHT_REL
const SHT_SHLIB          uint32 = C.SHT_SHLIB
const SHT_DYNSYM         uint32 = C.SHT_DYNSYM
const SHT_NUM            uint32 = C.SHT_NUM
const SHT_INIT_ARRAY     uint32 = C.SHT_INIT_ARRAY
const SHT_FINI_ARRAY     uint32 = C.SHT_FINI_ARRAY
const SHT_PREINIT_ARRAY  uint32 = C.SHT_PREINIT_ARRAY
const SHT_GROUP          uint32 = C.SHT_GROUP
const SHT_SYMTAB_SHNDX   uint32 = C.SHT_SYMTAB_SHNDX
const SHT_RELR           uint32 = C.SHT_RELR
const SHT_NUM_1          uint32 = 20
const SHT_LOOS           uint32 = C.SHT_LOOS
const SHT_HIOS           uint32 = C.SHT_HIOS
const SHT_ANDROID_REL    uint32 = C.SHT_ANDROID_REL
const SHT_ANDROID_RELA   uint32 = C.SHT_ANDROID_RELA
const SHT_ANDROID_RELR   uint32 = C.SHT_ANDROID_RELR
const SHT_LOPROC         uint32 = C.SHT_LOPROC
const SHT_HIPROC         uint32 = C.SHT_HIPROC
const SHT_LOUSER         uint32 = C.SHT_LOUSER
const SHT_HIUSER         uint32 = C.SHT_HIUSER

const SHF_WRITE          uint32 = C.SHF_WRITE
const SHF_ALLOC          uint32 = C.SHF_ALLOC
const SHF_EXECINSTR      uint32 = C.SHF_EXECINSTR
const SHF_RELA_LIVEPATCH uint32 = C.SHF_RELA_LIVEPATCH
const SHF_RO_AFTER_INIT  uint32 = C.SHF_RO_AFTER_INIT
const SHF_MASKPROC       uint32 = C.SHF_MASKPROC

const SHN_UNDEF          uint32 = C.SHN_UNDEF
const SHN_LORESERVE      uint32 = C.SHN_LORESERVE
const SHN_LOPROC         uint32 = C.SHN_LOPROC
const SHN_HIPROC         uint32 = C.SHN_HIPROC
const SHN_LIVEPATCH      uint32 = C.SHN_LIVEPATCH
const SHN_ABS            uint32 = C.SHN_ABS
const SHN_COMMON         uint32 = C.SHN_COMMON
const SHN_HIRESERVE      uint32 = C.SHN_HIRESERVE


type ElfSection struct {
    Name       string;
    NameIndex  uint32;
    Type       uint32;
    Flags      uint64;
    Addr       ElfAddr;
    Offset     ElfOffset;
    Size       ElfSize;
    Link       uint32;
    Info       uint32;
    AddrAlign  uint64;
    EntrySize  ElfSize;
}

func NewElfSection(sec *C.ElfSection) *ElfSection {
	section := &ElfSection {
		NameIndex:  uint32(sec.sh_name),
		Type:       uint32(sec.sh_type),
		Flags:		uint64(sec.sh_flags),
		Addr:		ElfAddr(sec.sh_addr),
		Offset:		ElfOffset(sec.sh_offset),
		Size:		ElfSize(sec.sh_size),
		Link:		uint32(sec.sh_link),
		Info:		uint32(sec.sh_info),
		AddrAlign:  uint64(sec.sh_addralign),
		EntrySize:  ElfSize(sec.sh_entsize),
	}
	return section;
}

// dynamic headers
const DT_NULL                 uint32 = C.DT_NULL
const DT_NEEDED               uint32 = C.DT_NEEDED
const DT_PLTRELSZ             uint32 = C.DT_PLTRELSZ
const DT_PLTGOT               uint32 = C.DT_PLTGOT
const DT_HASH                 uint32 = C.DT_HASH
const DT_STRTAB               uint32 = C.DT_STRTAB
const DT_SYMTAB               uint32 = C.DT_SYMTAB
const DT_RELA                 uint32 = C.DT_RELA
const DT_RELASZ               uint32 = C.DT_RELASZ
const DT_RELAENT              uint32 = C.DT_RELAENT
const DT_STRSZ                uint32 = C.DT_STRSZ
const DT_SYMENT               uint32 = C.DT_SYMENT
const DT_INIT                 uint32 = C.DT_INIT
const DT_FINI                 uint32 = C.DT_FINI
const DT_SONAME               uint32 = C.DT_SONAME
const DT_RPATH                uint32 = C.DT_RPATH
const DT_SYMBOLIC             uint32 = C.DT_SYMBOLIC
const DT_REL                  uint32 = C.DT_REL
const DT_RELSZ                uint32 = C.DT_RELSZ
const DT_RELENT               uint32 = C.DT_RELENT
const DT_PLTREL               uint32 = C.DT_PLTREL
const DT_DEBUG                uint32 = C.DT_DEBUG
const DT_TEXTREL              uint32 = C.DT_TEXTREL
const DT_JMPREL               uint32 = C.DT_JMPREL
const DT_BIND_NOW             uint32 = C.DT_BIND_NOW
const DT_INIT_ARRAY           uint32 = C.DT_INIT_ARRAY
const DT_FINI_ARRAY           uint32 = C.DT_FINI_ARRAY
const DT_INIT_ARRAYSZ         uint32 = C.DT_INIT_ARRAYSZ
const DT_FINI_ARRAYSZ         uint32 = C.DT_FINI_ARRAYSZ
const DT_RUNPATH              uint32 = C.DT_RUNPATH
const DT_FLAGS                uint32 = C.DT_FLAGS

/* glibc and BSD disagree for DT_ENCODING; glibc looks wrong. */
const DT_ENCODING             uint32 = C.DT_ENCODING
const DT_PREINIT_ARRAY        uint32 = C.DT_PREINIT_ARRAY
const DT_PREINIT_ARRAYSZ      uint32 = C.DT_PREINIT_ARRAYSZ
const DT_RELRSZ               uint32 = C.DT_RELRSZ
const DT_RELR                 uint32 = C.DT_RELR
const DT_RELRENT              uint32 = C.DT_RELRENT

const OLD_DT_LOOS             uint32 = C.OLD_DT_LOOS
const DT_LOOS                 uint32 = C.DT_LOOS
const DT_HIOS                 uint32 = C.DT_HIOS

const DT_ANDROID_REL          uint32 = C.DT_ANDROID_REL     // DT_LOOS + 2
const DT_ANDROID_RELSZ        uint32 = C.DT_ANDROID_RELSZ   // DT_LOOS + 3
const DT_ANDROID_RELA         uint32 = C.DT_ANDROID_RELA    // DT_LOOS + 4
const DT_ANDROID_RELASZ       uint32 = C.DT_ANDROID_RELASZ  // DT_LOOS + 5
const DT_ANDROID_RELR         uint32 = C.DT_ANDROID_RELR
const DT_ANDROID_RELRSZ       uint32 = C.DT_ANDROID_RELRSZ
const DT_ANDROID_RELRENT      uint32 = C.DT_ANDROID_RELRENT
const DT_ANDROID_RELRCOUNT    uint32 = C.DT_ANDROID_RELRCOUNT
const DT_VALRNGLO             uint32 = C.DT_VALRNGLO
const DT_VALRNGHI             uint32 = C.DT_VALRNGHI
const DT_ADDRRNGLO            uint32 = C.DT_ADDRRNGLO
const DT_GNU_HASH             uint32 = C.DT_GNU_HASH
const DT_TLSDESC_PLT          uint32 = C.DT_TLSDESC_PLT
const DT_TLSDESC_GOT          uint32 = C.DT_TLSDESC_GOT
const DT_ADDRRNGHI            uint32 = C.DT_ADDRRNGHI
const DT_VERSYM               uint32 = C.DT_VERSYM
const DT_RELACOUNT            uint32 = C.DT_RELACOUNT
const DT_RELCOUNT             uint32 = C.DT_RELCOUNT
const DT_FLAGS_1              uint32 = C.DT_FLAGS_1
const DT_VERDEF               uint32 = C.DT_VERDEF
const DT_VERDEFNUM            uint32 = C.DT_VERDEFNUM
const DT_VERNEED              uint32 = C.DT_VERNEED
const DT_VERNEEDNUM           uint32 = C.DT_VERNEEDNUM
const OLD_DT_HIOS             uint32 = C.OLD_DT_HIOS
const DT_LOPROC               uint32 = C.DT_LOPROC
const DT_HIPROC               uint32 = C.DT_HIPROC

type ElfDynamic struct {
	Tag   int64;
	Value uint64;
}

type ElfSymbol struct{
	Name       string
	NameIndex  uint32
	Info       byte
	Other      byte
	ShdrIndex  uint32
	Value      ElfAddr
	Size       ElfSize
}

func NewElfSymbol(sym *C.ElfSymbol) *ElfSymbol {
	symbol := &ElfSymbol {
		Name:       C.GoString(sym.sym_name),
		NameIndex:  uint32(sym.st_name),
		Info:       byte(sym.st_info),
		Other:      byte(sym.st_other),
		ShdrIndex:  uint32(sym.st_shndx),
		Value:      ElfAddr(sym.st_value),
		Size:       ElfSize(sym.st_size),
	}
	return symbol;
}

func (symbol *ElfSymbol) String() string {
	var result string = "";
	result = fmt.Sprintf("%s, %d, 0x%x, 0x%x, %d, %d, %d",
					symbol.Name,
					symbol.NameIndex,
					symbol.Value,
					symbol.Size,
					symbol.Info,
					symbol.Other,
					symbol.ShdrIndex);
	return result;
}

type ElfRel struct {
	Offset ElfOffset
	Info   uint64
}

type ElfRela struct {
	Offset ElfOffset
	Info   uint64
	AddEnd int64
}
