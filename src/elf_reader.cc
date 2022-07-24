
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>

#include "elf_log.h"
#include "elf_common.h"
#include "elf_reader.h"
#include "elf_image32.h"
#include "elf_image64.h"

elf_reader::elf_reader() {
    log_dbg("elf_reader ctor\n");
    m_loaded      = false;
    m_fd          = -1;
    m_file_size   = 0;
    m_load_bias   = 0;
    m_load_size   = 0;
    m_file_offset = 0;
}

elf_reader::~elf_reader() {
    log_dbg("elf_reader ctor\n");
    // do not close m_fd
    m_loaded      = false;
    m_fd          = -1;
}

bool elf_reader::open(const char * sopath) {
    ssize_t nread       = 0;
    uint8_t elf_class   = ELFCLASSNONE;
    uint8_t* ehdr       = (uint8_t*)&m_ehdr;

    int fd = ::open(sopath, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        log_error("open \"%s\" fail, error: %s\n", sopath, strerror(errno));
        return false;
    }
    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0) {
        log_error("get \"%s\" filesz fail, error: %s\n", sopath, strerror(errno));
        goto fail;
    }
    nread = pread(fd, ehdr, sizeof(Elf64_Ehdr), 0);
    if (nread != sizeof(Elf64_Ehdr)) {
        log_error("Read ELF Header length(%ld) fail, errno(%s)\n", nread, strerror(errno));
        goto fail;
    }
    if (ehdr[0] != 0x7F || 
        ehdr[1] != 'E' || ehdr[2] != 'L' || ehdr[3] != 'F') {
        log_error("Bad magic number for file");
        goto fail;
    }
    elf_class = ehdr[EI_CLASS];
    if (elf_class != ELFCLASS32 && elf_class != ELFCLASS64) {
        log_error("Unknonw ELF class (0x%02x)\n", elf_class);
        goto fail;
    }

    this->m_elf_class   = elf_class;
    this->m_loaded      = true;
    this->m_sopath      = sopath;
    this->m_soname      = basename((char*)sopath);
    this->m_fd          = fd;
    this->m_file_size   = file_stat.st_size;
    this->m_file_offset = 0;
    if (!this->verify_elf_header()) {
        goto fail;
    }
    if (!read_program_headers() ||
        !read_section_headers() ||
        !read_dynamic_section()) {
        goto fail;
    }
    return true;
fail:
    close();
    return false;
}

void elf_reader::close() {
    m_phdr_fragment.unmap();
    m_shdr_fragment.unmap();
    m_dynamic_fragment.unmap();
    m_dynstr_fragment.unmap();
    m_dynsym_fragment.unmap();
    m_strtab_fragment.unmap();
    m_symtab_fragment.unmap();
    m_shstrtab_fragment.unmap();
    if (m_fd != -1) {
        ::close(m_fd);
    }
    m_loaded      = false;
    m_fd          = -1;
}

elf_image* elf_reader::load_image() {

    dump_elf_header();
    dump_program_headers();
    dump_section_headers();
//    dump_sh_string();
//    dump_dynamics();
    size_t min_aligment = get_min_aligment((void*)m_phdr, m_phdr_num);
    if (min_aligment < PAGE_SIZE) {
        log_info("min_align(0x%lx), page_size(0x%lx), read_segment\n", min_aligment, (size_t)PAGE_SIZE);
        if (!read_segment()) {
            return NULL;
        }
    } else {
        log_info("min_align(0x%lx), page_size(0x%lx), load_segment\n", min_aligment, (size_t)PAGE_SIZE);
        if (!load_segment()) {
            return NULL;
        }
    }

    elf_image * image = NULL;
    if (this->get_elf_class() == ELFCLASS32) {
        image = new elf_image32(*this);
    } else if (this->get_elf_class() == ELFCLASS64) {
        image = new elf_image64(*this);
    }
    if (image && image->load()) {
        return image;
    }
    return NULL;
}

bool elf_reader::check_file_range(off_t offset, size_t size, size_t alignment) {
    off_t range_start;
    off_t range_end;
    return offset > 0 &&
        safe_add(&range_start, 0, offset) &&
        safe_add(&range_end, range_start, size) &&
        (range_start < m_file_size) &&
        (range_end <= m_file_size) &&
        ((offset % alignment) == 0);
}


bool elf_reader::verify_elf_header() {
    uint8_t *ident = (uint8_t*)&m_ehdr;
    uint8_t elf_class = ident[4];
    uint8_t elf_data = ident[5];
    
    if (elf_data != ELFDATA2LSB && elf_data != ELFDATA2MSB) {
        log_error("not little-endian or big-little\n");
        return false;
    }

    if (elf_class == ELFCLASS64) {
        Elf64_Ehdr *ehdr = &m_ehdr.ehdr64;
        if (ehdr->e_type != ET_DYN && ehdr->e_type != ET_EXEC) {
            log_error("unsupported ELF type(0x%04x)\n", ehdr->e_type);
            return false;
        }
        if (ehdr->e_machine != EM_AARCH64 && ehdr->e_machine != EM_X86_64)  {
            log_error("unsupported class64 cpu arch\n");
            return false;
        }
        if (sizeof(Elf64_Phdr) != ehdr->e_phentsize) {
            log_error("Bad Phdr Entry Size %d\n", ehdr->e_phentsize); 
            return false;
        }
        if (sizeof(Elf64_Shdr) != ehdr->e_shentsize) {
            log_error("Bad Shdr Entry Size %d\n", ehdr->e_shentsize); 
            return false;
        }
    } else if (elf_class == ELFCLASS32) {
        Elf32_Ehdr *ehdr = &m_ehdr.ehdr32;
        if (ehdr->e_type != ET_DYN && ehdr->e_type != ET_EXEC) {
            log_error("unsupported ELF type(0x%04x)\n", ehdr->e_type);
            return false;
        }
        if (ehdr->e_machine != EM_AARCH64 && ehdr->e_machine != EM_X86_64)  {
            log_error("unsupported class64 cpu arch\n");
            return false;
        }
        if (sizeof(Elf32_Phdr) != ehdr->e_phentsize) {
            log_error("Bad Phdr Entry Size %d\n", ehdr->e_phentsize); 
            return false;
        }
        if (sizeof(Elf32_Shdr) != ehdr->e_shentsize) {
            log_error("Bad Shdr Entry Size %d\n", ehdr->e_shentsize); 
            return false;
        }
    } else {
        log_error("unsupported ELF Class: %d\n", get_elf_class()); 
        return false;
    }
    return true;
}

bool elf_reader::read_program_headers() {
    size_t phdr_size        = 0;
    addr_t phdr_offset      = NULL;
    if (get_elf_class() == ELFCLASS64) {
        this->m_phdr_num = m_ehdr.ehdr64.e_phnum;
        phdr_offset = m_ehdr.ehdr64.e_phoff;
        phdr_size = sizeof(Elf64_Phdr);
    } else if (get_elf_class() == ELFCLASS32) {
        this->m_phdr_num = m_ehdr.ehdr32.e_phnum;
        phdr_offset = m_ehdr.ehdr32.e_phoff;
        phdr_size = sizeof(Elf32_Phdr);
    } else {
        log_error("unsupported ELF Class: %d\n", get_elf_class()); 
        return false;
    }
    
    if (this->m_phdr_num == 0) {
        log_error("\"%s\" has no program headers\n", this->get_sopath());
        return false;
    }
    if (this->m_phdr_num < 1 || this->m_phdr_num > (65536 / phdr_size)) {
        log_error("\"%s\" has invalid e_phnum: %lu\n", this->get_soname(), this->m_phdr_num);
        return false;
    }
    // Boundary checks
    size_t size = this->m_phdr_num * phdr_size;
    if (!check_file_range(phdr_offset, size, 4)) {
        log_error("\"%s\" has invalid phdr offset/size: %zu/%zu\n",
                this->get_soname(),
                static_cast<size_t>(phdr_offset),
                size);
        return false;
    }
    if (!this->m_phdr_fragment.map(this->m_fd, 0, phdr_offset, size)) {
        log_error("\"%s\" phdr mmap failed: %s\n", this->get_sopath(), strerror(errno));
        return false;
    }
    this->m_phdr = m_phdr_fragment.data();
    log_info("read program header: phdr(0x%p), phdr_num(%zu)\n", this->m_phdr, this->m_phdr_num);
    return true;
}


bool elf_reader::read_section_headers() {

    uint16_t shstrndx = 0;
    addr_t  shdr_offset = 0;
    size_t shdr_size = 0;
    if (get_elf_class() == ELFCLASS64) {
        this->m_shdr_num = m_ehdr.ehdr64.e_shnum;
        shstrndx         = m_ehdr.ehdr64.e_shstrndx;
        shdr_offset      = (addr_t)m_ehdr.ehdr64.e_shoff;
        shdr_size        = sizeof(Elf64_Shdr);
    } else if (get_elf_class() == ELFCLASS32) {
        this->m_shdr_num = m_ehdr.ehdr32.e_shnum;
        shstrndx         = m_ehdr.ehdr32.e_shstrndx;
        shdr_offset      = (addr_t)m_ehdr.ehdr32.e_shoff;
        shdr_size        = sizeof(Elf32_Shdr);
    } else {
        log_error("unsupported ELF Class: %d\n", get_elf_class()); 
        return false;
    }

    if (this->m_shdr_num == 0) {
        log_error("\"%s\" has no section headers\n", this->get_sopath());
        return false;
    }

    if (shstrndx >= this->m_shdr_num) {
        log_error("\"%s\" section headers nums less than e_shstrndx\n", this->get_sopath());
        return false;
    }

    size_t size = this->m_shdr_num * shdr_size;
    //log_dbg("filesize: %ld\n", m_file_size);
    if (!check_file_range(shdr_offset, size, 1)) {
        log_error("\"%s\" has invalid shdr offset/size: %zu/%zu\n",
                  this->get_sopath(),
                  (size_t)shdr_offset,
                  size);
        return false;
    }

    if (!this->m_shdr_fragment.map(this->m_fd, 0, shdr_offset, size)) {
        log_error("\"%s\" shdr mmap failed: %s\n", this->get_sopath(), strerror(errno));
        return false;
    }

    this->m_shdr = this->m_shdr_fragment.data();

    size_t shstrtab_size = 0;
    addr_t shstrstab_offset = 0;
    if (get_elf_class() == ELFCLASS64) {
        Elf64_Shdr* shstrtab_shdr = &((Elf64_Shdr*)this->m_shdr)[shstrndx];
        shstrstab_offset          = shstrtab_shdr->sh_offset;
        shstrtab_size             = shstrtab_shdr->sh_size;
    } else if (get_elf_class() == ELFCLASS32) {
        Elf32_Shdr* shstrtab_shdr = &((Elf32_Shdr*)this->m_shdr)[shstrndx];
        shstrstab_offset          = shstrtab_shdr->sh_offset;
        shstrtab_size             = shstrtab_shdr->sh_size;
    } else {
        log_error("unsupported ELF Class: %d\n", get_elf_class()); 
        return false;
    }

    if (!this->check_file_range(shstrstab_offset, shstrtab_size, 1)) {
       log_error("\"%s\" has invalid shdr offset/size: %zu/%zu\n",
                  this->get_sopath(),
                  (size_t)shstrstab_offset,
                  (size_t)shstrtab_size);
       return false;
    }
    if (!this->m_shstrtab_fragment.map(this->m_fd, 0, shstrstab_offset, shstrtab_size)) {
        log_error("\"%s\" shstrtab mmap failed: %s\n", this->get_sopath(), strerror(errno));
        return false;
    }
    this->m_shstrtab = (const char *)this->m_shstrtab_fragment.data();
    this->m_shstrtab_size = shstrtab_size;
    log_info("read section header: shdr(0x%p), shdr_num(%zu)\n", this->m_shdr, this->m_shdr_num);
    return true;
}


bool elf_reader::read_dynamic_section() {

    if (get_elf_class() == ELFCLASS64) {
        Elf64_Shdr * dynamic_shdr = NULL;
        Elf64_Shdr * dynsym_shdr = NULL;
        Elf64_Shdr * strtab_shdr = NULL;
        Elf64_Shdr * dynstr_shdr = NULL;
        Elf64_Shdr * symtab_shdr = NULL;
        Elf64_Shdr * shdr = (Elf64_Shdr*)this->m_shdr;
        for (size_t i = 0; i < this->m_shdr_num; ++i) {
            const char * sh_name = &this->m_shstrtab[shdr[i].sh_name];
            log_dbg("%-30s %d\n", sh_name, shdr[i].sh_type);
            if (shdr[i].sh_type == SHT_DYNAMIC) {
                dynamic_shdr = &shdr[i];
            } else if (shdr[i].sh_type == SHT_DYNSYM) {
                dynsym_shdr = &shdr[i];
            } else if (shdr[i].sh_type == SHT_STRTAB) {
                if (strncmp(sh_name, ".strtab", 7) == 0) {
                    strtab_shdr = &shdr[i];
                } else if (strncmp(sh_name, ".dynstr", 7) == 0) {
                    dynstr_shdr = &shdr[i];
                }
            } else if (shdr[i].sh_type == SHT_SYMTAB) {
                if (strncmp(sh_name, ".symtab", 7) == 0) {
                    symtab_shdr = &shdr[i];
                }
            }
        }

        if (dynamic_shdr)
            log_dbg(".dynamic %p, 0x%16llx, %zd\n", dynamic_shdr, (uint64_t)dynamic_shdr->sh_offset, (size_t)dynamic_shdr->sh_size);
        if (dynsym_shdr)
            log_dbg(".dynsym %p, 0x%16llx, %zd\n", dynsym_shdr,  (uint64_t)dynsym_shdr->sh_offset,  (size_t)dynsym_shdr->sh_size);
        if (dynstr_shdr)
            log_dbg(".dynstr %p, 0x%16llx, %zd\n", dynstr_shdr,  (uint64_t)dynstr_shdr->sh_offset,  (size_t)dynstr_shdr->sh_size);
        if (symtab_shdr)
            log_dbg(".symtab %p, 0x%16llx, %zd\n", symtab_shdr,  (uint64_t)symtab_shdr->sh_offset,  (size_t)symtab_shdr->sh_size);
        if (strtab_shdr)    
            log_dbg(".strtab %p, 0x%16llx, %zd\n", strtab_shdr,  (uint64_t)strtab_shdr->sh_offset,  (size_t)strtab_shdr->sh_size);
        
        if (dynamic_shdr && 
            check_file_range(dynamic_shdr->sh_offset, dynamic_shdr->sh_size, 4)) {
            if (!this->m_dynamic_fragment.map(this->m_fd, 0, dynamic_shdr->sh_offset, dynamic_shdr->sh_size)) {
                log_warn("dynamic map fail, %s\n", strerror(errno));
            }
            this->m_dynamic = (void *)this->m_dynamic_fragment.data();
            this->m_dynamic_size = dynamic_shdr->sh_size;
        }
        if (dynsym_shdr && 
            check_file_range(dynsym_shdr->sh_offset, dynsym_shdr->sh_size, 4)) {
            if (!this->m_dynsym_fragment.map(this->m_fd, 0, dynsym_shdr->sh_offset, dynsym_shdr->sh_size) ) {
                log_warn("dynsym map fail, %s\n", strerror(errno));
            }
            this->m_dynsym = (void *)this->m_dynsym_fragment.data();
            this->m_dynsym_size = dynsym_shdr->sh_size;
        }
        if (symtab_shdr && 
            check_file_range(symtab_shdr->sh_offset, symtab_shdr->sh_size, 4)) {
            if (!this->m_symtab_fragment.map(this->m_fd, 0, symtab_shdr->sh_offset, symtab_shdr->sh_size)) {
                log_warn("symtab map fail, %s\n", strerror(errno));
            }
            this->m_symtab = (void *)this->m_symtab_fragment.data();
            this->m_symtab_size = symtab_shdr->sh_size;
        }
        if (dynstr_shdr && 
            check_file_range(dynstr_shdr->sh_offset, dynstr_shdr->sh_size, 1)) {
            if (!this->m_dynstr_fragment.map(this->m_fd, 0, dynstr_shdr->sh_offset, dynstr_shdr->sh_size)) {
                log_warn("dynstr map fail, %s\n", strerror(errno));
            }
            this->m_dynstr = (const char *)this->m_dynstr_fragment.data();
            this->m_dynstr_size = dynstr_shdr->sh_size;
        }
        if (strtab_shdr && 
            check_file_range(strtab_shdr->sh_offset, strtab_shdr->sh_size, 1)) {
            if (!this->m_strtab_fragment.map(this->m_fd, 0, strtab_shdr->sh_offset, strtab_shdr->sh_size)) {
                log_warn("strtab map fail, %s\n", strerror(errno));
            }
            this->m_strtab = (const char *)this->m_strtab_fragment.data();
            this->m_strtab_size = strtab_shdr->sh_size; 
        }
        return true;
    } else if (get_elf_class() == ELFCLASS32) {
        Elf32_Shdr * dynamic_shdr = NULL;
        Elf32_Shdr * dynsym_shdr = NULL;
        Elf32_Shdr * strtab_shdr = NULL;
        Elf32_Shdr * dynstr_shdr = NULL;
        Elf32_Shdr * symtab_shdr = NULL;
        Elf32_Shdr * shdr = (Elf32_Shdr*)this->m_shdr;
        for (size_t i = 0; i < this->m_shdr_num; ++i) {
            const char * sh_name = &this->m_shstrtab[shdr[i].sh_name];
            log_dbg("%-30s %d\n", sh_name, shdr[i].sh_type);
            if (shdr[i].sh_type == SHT_DYNAMIC) {
                dynamic_shdr = &shdr[i];
            } else if (shdr[i].sh_type == SHT_DYNSYM) {
                dynsym_shdr = &shdr[i];
            } else if (shdr[i].sh_type == SHT_STRTAB) {
                if (strncmp(sh_name, ".strtab", 7) == 0) {
                    strtab_shdr = &shdr[i];
                } else if (strncmp(sh_name, ".dynstr", 7) == 0) {
                    dynstr_shdr = &shdr[i];
                }
            } else if (shdr[i].sh_type == SHT_SYMTAB) {
                if (strncmp(sh_name, ".symtab", 7) == 0) {
                    symtab_shdr = &shdr[i];
                }
            }
        }
        
        if (dynamic_shdr)
            log_dbg(".dynamic %p, 0x%08x, %zd\n", dynamic_shdr, (uint32_t)dynamic_shdr->sh_offset, (size_t)dynamic_shdr->sh_size);
        if (dynsym_shdr)
            log_dbg(".dynsym %p, 0x%08x, %zd\n", dynsym_shdr,  (uint32_t)dynsym_shdr->sh_offset,  (size_t)dynsym_shdr->sh_size);
        if (dynstr_shdr)
            log_dbg(".dynstr %p, 0x%08x, %zd\n", dynstr_shdr,  (uint32_t)dynstr_shdr->sh_offset,  (size_t)dynstr_shdr->sh_size);
        if (symtab_shdr)
            log_dbg(".symtab %p, 0x%08x, %zd\n", symtab_shdr,  (uint32_t)symtab_shdr->sh_offset,  (size_t)symtab_shdr->sh_size);
        if (strtab_shdr)    
            log_dbg(".strtab %p, 0x%08x, %zd\n", strtab_shdr,  (uint32_t)strtab_shdr->sh_offset,  (size_t)strtab_shdr->sh_size);

        if (dynamic_shdr && 
            check_file_range(dynamic_shdr->sh_offset, dynamic_shdr->sh_size, 4)) {
            if (!this->m_dynamic_fragment.map(this->m_fd, 0, dynamic_shdr->sh_offset, dynamic_shdr->sh_size)) {
                log_warn("dynamic map fail, %s\n", strerror(errno));
            }
            this->m_dynamic = (void*)this->m_dynamic_fragment.data();
            this->m_dynamic_size = dynamic_shdr->sh_size;
        }
        if (dynsym_shdr && 
            check_file_range(dynsym_shdr->sh_offset, dynsym_shdr->sh_size, 4)) {
            if (!this->m_dynsym_fragment.map(this->m_fd, 0, dynsym_shdr->sh_offset, dynsym_shdr->sh_size) ) {
                log_warn("dynsym map fail, %s\n", strerror(errno));
            }
            this->m_dynsym = (void *)this->m_dynsym_fragment.data();
            this->m_dynsym_size = dynsym_shdr->sh_size;
        }
        if (symtab_shdr && 
            check_file_range(symtab_shdr->sh_offset, symtab_shdr->sh_size, 4)) {
            if (!this->m_symtab_fragment.map(this->m_fd, 0, symtab_shdr->sh_offset, symtab_shdr->sh_size)) {
                log_warn("symtab map fail, %s\n", strerror(errno));
            }
            this->m_symtab = (void *)this->m_symtab_fragment.data();
            this->m_symtab_size = symtab_shdr->sh_size;
        }
        if (dynstr_shdr && 
            check_file_range(dynstr_shdr->sh_offset, dynstr_shdr->sh_size, 1)) {
            if (!this->m_dynstr_fragment.map(this->m_fd, 0, dynstr_shdr->sh_offset, dynstr_shdr->sh_size)) {
                log_warn("dynstr map fail, %s\n", strerror(errno));
            }
            this->m_dynstr = (const char *)this->m_dynstr_fragment.data();
            this->m_dynstr_size = dynstr_shdr->sh_size;
        }
        if (strtab_shdr && 
            check_file_range(strtab_shdr->sh_offset, strtab_shdr->sh_size, 1)) {
            if (!this->m_strtab_fragment.map(this->m_fd, 0, strtab_shdr->sh_offset, strtab_shdr->sh_size)) {
                log_warn("strtab map fail, %s\n", strerror(errno));
            }
            this->m_strtab = (const char *)this->m_strtab_fragment.data();
            this->m_strtab_size = strtab_shdr->sh_size; 
        }
        return true;
    } 
    log_error("unsupported ELF Class: %d", get_elf_class()); 
    return false;
}


size_t elf_reader::get_load_size(void* phdr, size_t phdr_num, addr_t* out_min_vaddr, addr_t* out_max_vaddr) {
    uint8_t * p = (uint8_t*)phdr;
    addr_t min_vaddr = UINTPTR_MAX;
    addr_t max_vaddr = 0;
    bool found_pt_load = false;
    assert(get_elf_class() == ELFCLASS32 || get_elf_class() == ELFCLASS64); 

    for (int i = 0; i < phdr_num; i++) {
        addr_t p_vaddr;
        addr_t p_memsz;
        uint32_t p_type;
        if (get_elf_class() == ELFCLASS32) {
            Elf32_Phdr *phdr32 = (Elf32_Phdr*)p;
            p_type             = phdr32->p_type;
            p_vaddr            = (uint64_t)phdr32->p_vaddr;
            p_memsz            = (uint64_t)phdr32->p_memsz;
            p += sizeof(Elf32_Phdr);
        } else if (get_elf_class() == ELFCLASS64) {
            Elf64_Phdr *phdr64 = (Elf64_Phdr*)p;
            p_type             = phdr64->p_type;
            p_vaddr            = (uint64_t)phdr64->p_vaddr;
            p_memsz            = (uint64_t)phdr64->p_memsz;
            p += sizeof(Elf64_Phdr);
        } 
        if (p_type != PT_LOAD) {
            continue;
        }
        found_pt_load = true;
        if (p_vaddr < min_vaddr) {
            min_vaddr = p_vaddr;
        }

        if (p_vaddr + p_memsz > max_vaddr) {
            max_vaddr = p_vaddr + p_memsz;
        }
    }
    if (!found_pt_load) {
        min_vaddr = 0;
    }

    min_vaddr = PAGE_START(min_vaddr);
    max_vaddr = PAGE_END(max_vaddr);

    if (out_min_vaddr != NULL) {
        *out_min_vaddr = min_vaddr;
    }
    if (out_max_vaddr != NULL) {
        *out_max_vaddr = max_vaddr;
    }
    return max_vaddr - min_vaddr;
}

size_t elf_reader::get_max_aligment(void* phdr, size_t phdr_num) {
    uint8_t * p = (uint8_t*)phdr;
    size_t max_aligment = PAGE_SIZE;
    uint32_t p_type;
    assert(get_elf_class() == ELFCLASS32 || get_elf_class() == ELFCLASS64);
    for (int i = 0; i < phdr_num; i++) {
        uint64_t p_align;
        if (get_elf_class() == ELFCLASS32) {
            Elf32_Phdr *phdr32 = (Elf32_Phdr*)p;
            p_type             = phdr32->p_type;
            p_align            = (uint64_t)phdr32->p_align;
            p += sizeof(Elf32_Phdr);
        } else if (get_elf_class() == ELFCLASS64) {
            Elf64_Phdr *phdr64 = (Elf64_Phdr*)p;
            p_type             = phdr64->p_type;
            p_align            = (uint64_t)phdr64->p_align;
            p += sizeof(Elf64_Phdr);
        } 
        if (p_type != PT_LOAD || (p_align & (p_align - 1)) != 0) {
            continue;
        }
        if (p_align > max_aligment) {
            max_aligment = p_align;
        }
    }
    return max_aligment;
}

size_t elf_reader::get_min_aligment(void* phdr, size_t phdr_num) {
    uint8_t * p = (uint8_t*)phdr;
    size_t min_aligment = 0x400000;
    uint32_t p_type;
    assert(get_elf_class() == ELFCLASS32 || get_elf_class() == ELFCLASS64);
    for (int i = 0; i < phdr_num; i++) {
        uint64_t p_align;
        if (get_elf_class() == ELFCLASS32) {
            Elf32_Phdr *phdr32 = (Elf32_Phdr*)p;
            p_type             = phdr32->p_type;
            p_align            = (uint64_t)phdr32->p_align;
            p += sizeof(Elf32_Phdr);
        } else if (get_elf_class() == ELFCLASS64) {
            Elf64_Phdr *phdr64 = (Elf64_Phdr*)p;
            p_type             = phdr64->p_type;
            p_align            = (uint64_t)phdr64->p_align;
            p += sizeof(Elf64_Phdr);
        } 
        if (p_type != PT_LOAD || (p_align & (p_align - 1)) != 0) {
            continue;
        }
        if (p_align < min_aligment) {
            min_aligment = p_align;
        }
    }
    return min_aligment;
}

bool elf_reader::read_segment(void) {
    addr_t p_min_addr = (addr_t)NULL;
    addr_t p_max_addr = (addr_t)NULL;

    size_t load_size = get_load_size((void*)m_phdr, m_phdr_num, &p_min_addr, &p_max_addr);
    assert(load_size > 0);
    
    void* mmap_ptr = mmap(nullptr, load_size,  PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mmap_ptr == MAP_FAILED) {
        log_error("reserve address space fail, load(0x%zu), error(%s)\n", load_size, strerror(errno));
        return false;
    }
    m_load_bias = (addr_t)mmap_ptr;
    m_load_size = load_size;

    log_info("m_load_bias: 0x%llx, load_size: 0x%lx\n", m_load_bias, m_load_size);
    for (int i = 0; i < m_phdr_num; ++i) {
        uint32_t p_type;
        addr_t p_vaddr;
        addr_t p_offset;
        size_t p_align;
        size_t p_filesz;
        if (get_elf_class() == ELFCLASS64) {
            const Elf64_Phdr* phdr = &((Elf64_Phdr*)m_phdr)[i];
            p_type    = (uint32_t)phdr->p_type;
            p_vaddr   = (addr_t)phdr->p_vaddr;
            p_offset  = (addr_t)phdr->p_offset;
            p_filesz  = (size_t)phdr->p_filesz;
            p_align   = (size_t)phdr->p_align;
        } else if (get_elf_class() == ELFCLASS32) {
            const Elf32_Phdr* phdr = &((Elf32_Phdr*)m_phdr)[i];
            p_type     = (uint32_t)phdr->p_type;
            p_vaddr    = (addr_t)phdr->p_vaddr;
            p_offset   = (addr_t)phdr->p_offset;
            p_filesz   = (size_t)phdr->p_filesz;
            p_align   = (size_t)phdr->p_align;
        }
        if (p_type != PT_LOAD || p_filesz <= 0) {
            continue;
        }

        size_t nread = pread(m_fd, (void*)(m_load_bias + p_vaddr), p_filesz, (off_t)p_offset);
        log_dbg("read segment: i(%d), addr(%p), offset(%p), filesz(%lx), nread(%lx)\n", 
                i, 
                (void*)(m_load_bias + p_vaddr),
                (void*)p_offset,
                p_filesz,
                nread);

        if (nread != p_filesz) {
            log_error("couldn't read \"%s\" segment %d: %s\n", m_soname.c_str(), i, strerror(errno));
            return false;
        }
    }
    return true;
}
bool elf_reader::load_segment(void) {

    addr_t p_min_addr = (addr_t)NULL;
    addr_t p_max_addr = (addr_t)NULL;
      
    size_t load_size = get_load_size((void*)m_phdr, m_phdr_num, &p_min_addr, &p_max_addr);
    assert(load_size > 0);

    void* mmap_ptr = mmap(nullptr, load_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mmap_ptr == MAP_FAILED) {
        log_error("reserve address space fail, load(0x%zu), error(%s)\n", load_size, strerror(errno));
        return false;
    }
    m_load_bias = (addr_t)mmap_ptr;
    m_load_size = load_size;


    log_info("m_load_bias: 0x%llx, load_size: 0x%lx\n", m_load_bias, m_load_size);
    for (int i = 0; i < m_phdr_num; ++i) {
        uint32_t p_type;
        addr_t p_vaddr;
        addr_t p_offset;
        size_t p_align;
        uint64_t p_memsz;
        uint64_t p_filesz;
        if (get_elf_class() == ELFCLASS64) {
            const Elf64_Phdr* phdr = &((Elf64_Phdr*)m_phdr)[i];
            p_type    = (uint32_t)phdr->p_type;
            p_vaddr   = (addr_t)phdr->p_vaddr;
            p_offset  = (addr_t)phdr->p_offset;
            p_memsz   = (uint64_t)phdr->p_memsz;
            p_filesz  = (uint64_t)phdr->p_filesz;
            p_align   = (size_t)phdr->p_align;
        } else if (get_elf_class() == ELFCLASS32) {
            const Elf32_Phdr* phdr = &((Elf32_Phdr*)m_phdr)[i];
            p_type     = (uint32_t)phdr->p_type;
            p_vaddr    = (addr_t)phdr->p_vaddr;
            p_offset   = (addr_t)phdr->p_offset;
            p_memsz    = (uint64_t)phdr->p_memsz;
            p_filesz   = (uint64_t)phdr->p_filesz;
            p_align   = (size_t)phdr->p_align;
        }
        if (p_type != PT_LOAD) {
            continue;
        }


        addr_t seg_start       = p_vaddr + m_load_bias;
        addr_t seg_end         = seg_start + p_memsz;
        addr_t seg_page_start  = PAGE_START(seg_start);
        addr_t seg_page_end    = PAGE_END(seg_end);

        addr_t seg_file_end    = seg_start + p_filesz;

        // File offsets.
        addr_t file_start      = p_offset;
        addr_t file_end        = file_start + p_filesz;
        addr_t file_page_start = PAGE_START(file_start);
        addr_t file_length     = file_end - file_page_start;

        void* seg_addr = mmap((void*)seg_start,
                                file_length,
                                PROT_EXEC | PROT_READ,
                                MAP_FIXED | MAP_PRIVATE,
                                this->m_fd,
                                this->m_file_offset + file_start);

        log_dbg("load segment: i(%d), addr(%p), seg_start(%p), seg_page_start(%p), file_start(%p), file_page_start(%p)\n", 
                        i, 
                        seg_addr,
                        (void*)seg_start,
                        (void*)seg_page_start,
                        (void*)file_start,
                        (void*)file_page_start);

        if (seg_addr == MAP_FAILED) {
            log_error("couldn't map \"%s\" segment %d: %s\n", m_soname.c_str(), i, strerror(errno));
            return false;
        }
    }
    return true;
}


addr_t elf_reader::page_start(addr_t addr, size_t page_size) {
    return (addr_t)((~(page_size - 1)) & (addr));
}
addr_t elf_reader::page_end(addr_t addr, size_t page_size) {
    return (addr_t)(PAGE_START((addr) + (page_size - 1)));
}
off_t elf_reader::page_offset(addr_t addr, size_t page_size) {
    return (off_t)((addr) & (page_size - 1));
}

void elf_reader::dump_elf_header(void) {
    static char alpha_tab[17] = "0123456789ABCDEF";
    char buff[EI_NIDENT*3+1];
    uint8_t* p = (uint8_t*)&m_ehdr;
    log_info("ELF%d Header :\n", get_elf_class() == ELFCLASS64 ? 64 : 32);
    for(int i = 0; i < EI_NIDENT; i++) {
        uint8_t ch = p[i];
        buff[i*3 + 0] = alpha_tab[(int)((ch >> 4) & 0x0F)];
        buff[i*3 + 1] = alpha_tab[(int)(ch & 0x0F)];
        buff[i*3 + 2] = ' ';
    }
    buff[EI_NIDENT*3] = '\0';
    log_info("e_ident:     %s\n",         buff);

    if (get_elf_class() == ELFCLASS64) {
        Elf64_Ehdr* ehdr = &this->m_ehdr.ehdr64;
        log_info("e_type:      0x%x\n",       ehdr->e_type);
        log_info("e_machine:   0x%x\n",       ehdr->e_machine);
        log_info("e_version:   0x%x\n",       ehdr->e_version);
        log_info("e_entry:     0x%lx\n",      (unsigned long)ehdr->e_entry);
        log_info("e_phoff:     0x%lx\n",      (unsigned long)ehdr->e_phoff);
        log_info("e_shoff:     0x%lx\n",      (unsigned long)ehdr->e_shoff);
        log_info("e_flags:     0x%x\n",       ehdr->e_flags);
        log_info("e_ehsize:    0x%x\n",       ehdr->e_ehsize);
        log_info("e_phentsize: 0x%x\n",       ehdr->e_phentsize);
        log_info("e_phnum:     0x%x\n",       ehdr->e_phnum);
        log_info("e_shentsize: 0x%x\n",       ehdr->e_shentsize);
        log_info("e_shnum:     0x%x\n",       ehdr->e_shnum);
        log_info("e_shstrndx:  0x%x\n",       ehdr->e_shstrndx);
    } else if (get_elf_class() == ELFCLASS32) {
        Elf32_Ehdr* ehdr = &this->m_ehdr.ehdr32;
        log_info("e_type:      0x%x\n",       ehdr->e_type);
        log_info("e_machine:   0x%x\n",       ehdr->e_machine);
        log_info("e_version:   0x%x\n",       ehdr->e_version);
        log_info("e_entry:     0x%lx\n",      (unsigned long)ehdr->e_entry);
        log_info("e_phoff:     0x%lx\n",      (unsigned long)ehdr->e_phoff);
        log_info("e_shoff:     0x%lx\n",      (unsigned long)ehdr->e_shoff);
        log_info("e_flags:     0x%x\n",       ehdr->e_flags);
        log_info("e_ehsize:    0x%x\n",       ehdr->e_ehsize);
        log_info("e_phentsize: 0x%x\n",       ehdr->e_phentsize);
        log_info("e_phnum:     0x%x\n",       ehdr->e_phnum);
        log_info("e_shentsize: 0x%x\n",       ehdr->e_shentsize);
        log_info("e_shnum:     0x%x\n",       ehdr->e_shnum);
        log_info("e_shstrndx:  0x%x\n",       ehdr->e_shstrndx);
    }
    return;
}
void elf_reader::dump_program_headers(void) {
    log_info("ELF%d program headers: phdr_num(%d)\n", get_elf_class() == ELFCLASS64 ? 64 : 32, (int)this->m_phdr_num);
    
    if (get_elf_class() == ELFCLASS64) {
        Elf64_Phdr *phdr = (Elf64_Phdr*)this->m_phdr;
        log_info("[Index]     Type          Offset       VirtAddrs          PhyAddrs            FileSize  MemSize   Align     Flags\n");
        for (int i = 0; i < this->m_phdr_num; i++) {
            log_info("[%.2d]        %-12s  0x%0.8lx  0x%0.16lx  0x%0.16lx  %0.8lx  %0.8lx  %0.8lx  %0.4x\n",
                     i,
                     elf_phdr_type_name((int)(phdr[i].p_type)),
                     (unsigned long)phdr[i].p_offset,
                     (unsigned long)phdr[i].p_vaddr,
                     (unsigned long)phdr[i].p_paddr,
                     (unsigned long)phdr[i].p_filesz,
                     (unsigned long)phdr[i].p_memsz,
                     (unsigned long)phdr[i].p_align,
                     (unsigned int)phdr[i].p_flags);
        }
    } else if (get_elf_class() == ELFCLASS32) {
        Elf32_Phdr *phdr = (Elf32_Phdr*)this->m_phdr;
        log_info("[Index]     Type          Offset       VirtAddrs          PhyAddrs            FileSize  MemSize   Flags\n");
        for (int i = 0; i < this->m_phdr_num; i++) {

            log_info("[%.2d]        %-12s  0x%0.8lx  0x%0.8lx  0x%0.8lx  %0.8lx  %0.8lx  %0.8lx  %0.4x\n",
                     i,
                     elf_phdr_type_name((int)(phdr[i].p_type)),
                     (unsigned long)phdr[i].p_offset,
                     (unsigned long)phdr[i].p_vaddr,
                     (unsigned long)phdr[i].p_paddr,
                     (unsigned long)phdr[i].p_filesz,
                     (unsigned long)phdr[i].p_memsz,
                     (unsigned long)phdr[i].p_align,
                     (unsigned int)phdr[i].p_flags);
        }
    }
    return;
}

void elf_reader::dump_section_headers() {
    
    size_t shnum = this->m_shdr_num;
    log_info("ELF%d Section headers: shdr_num(%lu)\n", get_elf_class() == ELFCLASS64 ? 64 : 32, shnum);
    
    if (get_elf_class() == ELFCLASS64) {
        Elf64_Shdr * shdr = (Elf64_Shdr*)this->m_shdr;
        log_info("[Index]   Type          NameIndex   Address             Offset      Size        ES      Flags     Link\n");
        for(int i = 0; i < shnum; i += 1, shdr += 1) {
            log_info("[%0.2d]      %-12s  0x%0.8x  0x%0.16lx  0x%0.8lx  0x%0.8lu  0x%0.4lx  0x%0.4lx    0x%0.4lx\n",
                i,
                elf_shdr_type_name((int)shdr->sh_type),
                (int)shdr->sh_name,
                (unsigned long)shdr->sh_addr,
                (unsigned long)shdr->sh_offset,
                (unsigned long)shdr->sh_size,
                (unsigned long)shdr->sh_entsize,
                (unsigned long)shdr->sh_flags,
                (unsigned long)shdr->sh_link);
        }
    } else if (get_elf_class() == ELFCLASS32) {
        Elf32_Shdr * shdr = (Elf32_Shdr*)this->m_shdr;
        log_info("[Index]   Type          NameIndex   Address             Offset      Size        ES      Flags     Link\n");
        for(int i = 0; i < shnum; i += 1, shdr += 1) {
            log_info("[%0.2d]      %-12s  0x%0.8x  0x%0.8lx  0x%0.8lx  0x%0.8lu  0x%0.4lx  0x%0.4lx    0x%0.4lx\n",
                i,
                elf_shdr_type_name((int)shdr->sh_type),
                (int)shdr->sh_name,
                (unsigned long)shdr->sh_addr,
                (unsigned long)shdr->sh_offset,
                (unsigned long)shdr->sh_size,
                (unsigned long)shdr->sh_entsize,
                (unsigned long)shdr->sh_flags,
                (unsigned long)shdr->sh_link);
        }
    }

}

void elf_reader::dump_dynamics(void) {
    size_t dyn_num = this->m_dynamic_size;
    
    log_info("ELF64 dynamic headers: dyn_num(%lu)\n", dyn_num);
    if (get_elf_class() == ELFCLASS64) {
        Elf64_Dyn *dyn = (Elf64_Dyn*)this->m_dynamic;

        log_info("[Index]     TYPE      VALUE\n");
        for(int i = 0; i < dyn_num; i += 1, dyn += 1) {
            log_info("[%.2d] %-12s     0x%0.16lx\n",
                        i,
                        elf_dynamic_tag_name(dyn->d_tag),
                        (unsigned long)dyn->d_un.d_val);
            if(dyn->d_tag == DT_NULL) {
                break;
            }
        }
    } else if (get_elf_class() == ELFCLASS32) {
        Elf32_Dyn *dyn = (Elf32_Dyn*)this->m_dynamic;

        log_info("[Index]     TYPE      VALUE\n");
        for(int i = 0; i < dyn_num; i += 1, dyn += 1) {
            log_info("[%.2d] %-12s     0x%0.8lx\n",
                        i,
                        elf_dynamic_tag_name(dyn->d_tag),
                        (unsigned long)dyn->d_un.d_val);
            if(dyn->d_tag == DT_NULL) {
                break;
            }
        }
    }
    
    return;
}

void elf_reader::dump_sh_string() {
    const char * p = this->m_shstrtab;
    const char * end = p + this->m_shstrtab_size;
    int i = 0;
    while(p < end) {
        if (*p != '\0') {
            size_t s = strlen(p);
            if (p + s > end) {
                break;
            }
            log_info("[%0.2d]   %lu  %s \n", i, s, p);
            i += 1;
            p = p + s;
        } else {
            p = p + 1;
        }
    } 
}

