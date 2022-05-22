
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
#include "elf_image.h"
#include "elf_mapped.h"

elf_image * elf_image::create(const char * sopath) {
    Elf64_Ehdr ehdr;
    ssize_t nread;
    elf_image * image = NULL;
    int fd = open(sopath, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        log_error("open \"%s\" fail, error: %s\n", sopath, strerror(errno));
        return NULL;
    }
    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0) {
        log_error("get \"%s\" filesz fail, error: %s\n", sopath, strerror(errno));
        goto fail;
    }
    
    memset(&ehdr, 0, sizeof(ehdr));
    nread = pread(fd, &ehdr, sizeof(ehdr), 0);
    if (nread != sizeof(ehdr)) {
        log_error("read ELF file head fail, nread:%lu, error:%s\n", nread, strerror(errno));
        goto fail;
    }
log_dbg("asdfasdfadsf\n");
    if (ehdr.e_machine == EM_ARM) {
        elf_image32 * img = new elf_image32();
        memcpy(&img->m_ehdr, &ehdr, sizeof(ehdr));
        image = img;
    } else if (ehdr.e_machine == EM_AARCH64) {
        elf_image64 * img = new elf_image64();
        memcpy(&img->m_ehdr, &ehdr, sizeof(ehdr));
        image = img;
    } else if (ehdr.e_machine == EM_386) {
        log_error("unsupport x86");
        goto fail;
    } else if (ehdr.
        e_machine == EM_X86_64) {
        log_error("unsupport x86_64");
        goto fail;
    }
    image->m_loaded    = true;
    image->m_sopath    = sopath;
    image->m_soname    = basename((char*)sopath);
    image->m_fd        = fd;
    image->m_file_size = file_stat.st_size;
    
    return image;
fail:
    if (fd >= 0) {
        close(fd);

    }
    return NULL;
}

elf_image::elf_image() {
    m_loaded    = false;
    m_fd        = -1;
    m_file_size = 0;
    m_base_addr = 0;
}

elf_image::~elf_image() {

}

bool elf_image::load() {
    log_dbg("load...\n");
    if (!this->parse_program_headers() ||
        !this->parse_section_headers() || 
        !this->parse_sections())  {
        return false;
    }
    return true;
}

bool elf_image::check_file_range(off64_t offset, size_t size, size_t alignment) {
    off64_t range_start;
    off64_t range_end;
    return offset > 0 &&
        safe_add(&range_start, 0, offset) &&
        safe_add(&range_end, range_start, size) &&
        (range_start < m_file_size) &&
        (range_end <= m_file_size) &&
        ((offset % alignment) == 0);
}

elf_image64::elf_image64() {
    log_dbg("elf_image64 ctor, this: %p\n", this);
}
elf_image64::~elf_image64() {
    log_dbg("elf_image64 dtor, this: %p\n", this);
}
bool elf_image64::parse_program_headers() {
    log_dbg("parse_program_headers\n");
    this->m_phdr_num = m_ehdr.e_phnum;
    if (this->m_phdr_num == 0) {
        log_error("\"%s\" has no program headers\n", this->get_sopath());
        return false;
    }
    if (this->m_phdr_num < 1 || this->m_phdr_num > 65536/sizeof(Elf64_Phdr)) {
        log_error("\"%s\" has invalid e_phnum: %lu\n", this->get_soname(), this->m_phdr_num);
        return false;
    }
    // Boundary checks
    size_t size = this->m_phdr_num * sizeof(Elf64_Phdr);
    if (!check_file_range(this->m_ehdr.e_phoff, size, 4)) {
        log_error("\"%s\" has invalid phdr offset/size: %zu/%zu\n",
                this->get_soname(),
                static_cast<size_t>(this->m_ehdr.e_phoff),
                size);
        return false;
    }
    log_dbg("parse_program_headers map >>>>\n");
    if (!this->m_phdr_fragment.map(this->m_fd, 0, m_ehdr.e_phoff, size)) {
        log_error("\"%s\" phdr mmap failed: %s\n", this->get_sopath(), strerror(errno));
        return false;
    }
    log_dbg("parse_program_headers map <<<n");
    this->m_phdr = (Elf64_Phdr *)m_phdr_fragment.data();
    return true;
}

bool elf_image64::parse_section_headers() {
    this->m_shdr_num = this->m_ehdr.e_shnum;
    if (this->m_shdr_num == 0) {
        log_error("\"%s\" has no section headers\n", this->get_sopath());
        return false;
    }

    if (this->m_ehdr.e_shstrndx >= this->m_shdr_num) {
        log_error("\"%s\" section headers nums less than e_shstrndx\n", this->get_sopath());
        return false;
    }

    size_t size = this->m_shdr_num * sizeof(Elf64_Shdr);
    if (!check_file_range(this->m_ehdr.e_shoff, size, 4)) {
        log_error("\"%s\" has invalid shdr offset/size: %zu/%zu\n",
                  this->get_sopath(),
                  (size_t)this->m_ehdr.e_shoff,
                  size);
        return false;
    }

    if (!this->m_shdr_fragment.map(this->m_fd, 0, this->m_ehdr.e_shoff, size)) {
        log_error("\"%s\" shdr mmap failed: %s\n", this->get_sopath(), strerror(errno));
        return false;
    }

    this->m_shdr = (Elf64_Shdr *)this->m_shdr_fragment.data();

    Elf64_Shdr * shstrtab_shdr = &this->m_shdr[this->m_ehdr.e_shstrndx];
    if (!this->check_file_range(shstrtab_shdr->sh_offset, shstrtab_shdr->sh_size, 1)) {
       log_error("\"%s\" has invalid shdr offset/size: %zu/%zu\n",
                  this->get_sopath(),
                  (size_t)this->m_ehdr.e_shoff,
                  size);
       return false;
    }
    if (!this->m_shstrtab_fragment.map(this->m_fd, 0, shstrtab_shdr->sh_offset, shstrtab_shdr->sh_size)) {
        log_error("\"%s\" shstrtab mmap failed: %s\nm", this->get_sopath(), strerror(errno));
        return false;
    }
    this->m_shstrtab = (const char *)this->m_shstrtab_fragment.data();
    this->m_shstrtab_size = shstrtab_shdr->sh_size;
    return true;
}

bool elf_image64::parse_sections() {
    Elf64_Shdr * dynamic_shdr = NULL;
    Elf64_Shdr * dynsym_shdr = NULL;
    Elf64_Shdr * strtab_shdr = NULL;
    Elf64_Shdr * dynstr_shdr = NULL;
    Elf64_Shdr * symtab_shdr = NULL;

    for (size_t i = 0; i < this->m_shdr_num; ++i) {
        const char * sh_name = &this->m_shstrtab[this->m_shdr[i].sh_name];
    //    log_dbg("%-30s %d\n", sh_name, this->m_shdr[i].sh_type);
        if (this->m_shdr[i].sh_type == SHT_DYNAMIC) {
            dynamic_shdr = &this->m_shdr[i];
        } else if (this->m_shdr[i].sh_type == SHT_DYNSYM) {
            dynsym_shdr = &this->m_shdr[i];
        } else if (this->m_shdr[i].sh_type == SHT_STRTAB) {
            if (strncmp(sh_name, ".strtab", 7) == 0) {
                strtab_shdr = &this->m_shdr[i];
            } else if (strncmp(sh_name, ".dynstr", 7) == 0) {
                dynstr_shdr = &this->m_shdr[i];
            }
        } else if (this->m_shdr[i].sh_type == SHT_SYMTAB) {
            if (strncmp(sh_name, ".symtab", 7) == 0) {
                symtab_shdr = &this->m_shdr[i];
            }
        }
    }

    if (dynamic_shdr)
        log_dbg(".dynamic %p, %p, %zd\n", (void*)dynamic_shdr, (void*)dynamic_shdr->sh_offset, (size_t)dynamic_shdr->sh_size);
    if (dynsym_shdr)
        log_dbg(".dynsym  %p, %p, %zd\n", (void*)dynsym_shdr,  (void*)dynsym_shdr->sh_offset,  (size_t)dynsym_shdr->sh_size);
    if (dynstr_shdr)
        log_dbg(".dynstr  %p, %p, %zd\n", (void*)dynstr_shdr,  (void*)dynstr_shdr->sh_offset,  (size_t)dynstr_shdr->sh_size);
    if (symtab_shdr)
        log_dbg(".symtab  %p, %p, %zd\n", (void*)symtab_shdr,  (void*)symtab_shdr->sh_offset,  (size_t)symtab_shdr->sh_size);
    if (strtab_shdr)    
        log_dbg(".strtab  %p, %p, %zd\n", (void*)strtab_shdr,  (void*)strtab_shdr->sh_offset,  (size_t)strtab_shdr->sh_size);
    
    if (dynamic_shdr && 
        check_file_range(dynamic_shdr->sh_offset, dynamic_shdr->sh_size, 4)) {
        if (!this->m_dynamic_fragment.map(this->m_fd, 0, dynamic_shdr->sh_offset, dynamic_shdr->sh_size)) {
            log_warn("dynamic map fail, %s\n", strerror(errno));
        }
        this->m_dynamic = (Elf64_Dyn *)this->m_dynamic_fragment.data();
        this->m_dynamic_size = dynamic_shdr->sh_size;
    }
    if (dynsym_shdr && check_file_range(dynsym_shdr->sh_offset, dynsym_shdr->sh_size, 4)) {
        if (!this->m_dynsym_fragment.map(this->m_fd, 0, dynsym_shdr->sh_offset, dynsym_shdr->sh_size) ) {
            log_warn("dynsym map fail, %s\n", strerror(errno));
        }
        this->m_dynsym = (Elf64_Sym *)this->m_dynsym_fragment.data();
        this->m_dynsym_size = dynsym_shdr->sh_size;
    }
    if (symtab_shdr && 
        check_file_range(symtab_shdr->sh_offset, symtab_shdr->sh_size, 4)) {
        if (!this->m_symtab_fragment.map(this->m_fd, 0, symtab_shdr->sh_offset, symtab_shdr->sh_size)) {
            log_warn("symtab map fail, %s\n", strerror(errno));
        }
        this->m_symtab = (Elf64_Sym *)this->m_symtab_fragment.data();
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

elf_image32::elf_image32() {
    log_dbg("elf_image32 ctor, this: %p", this);
}
elf_image32::~elf_image32() {
    log_dbg("elf_image32 dtor, this: %p", this);
}

bool elf_image32::parse_program_headers() {
    return false;
}
bool elf_image32::parse_section_headers() {
    return false;
}
bool elf_image32::parse_sections() {
    return false;
}
