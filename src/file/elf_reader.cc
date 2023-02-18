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
#include <assert.h>

#include <common/elf.h>
#include <common/elf_log.h>
#include <file/elf_reader.h>
#include <image/elf_image.h>
#include <image/elf_image32.h>
#include <image/elf_image64.h>


bool elf_reader::open(const char * sopath) {
    ssize_t  nread       = 0;
    uint8_t  elf_class   = ELFCLASSNONE;
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
        log_error("read elf header length(%ld) fail, error: %s\n", nread, strerror(errno));
        goto fail;
    }
    if (ehdr[0] != 0x7F || 
        ehdr[1] != 'E' || ehdr[2] != 'L' || ehdr[3] != 'F') {
        log_error("bad magic number for file \"%s\"\n", sopath);
        goto fail;
    }
    elf_class = ehdr[EI_CLASS];
    if (elf_class != ELFCLASS32 && elf_class != ELFCLASS64) {
        log_error("unknonw elf class (0x%02x)\n", elf_class);
        goto fail;
    }
    this->m_elf_class   = elf_class;
    this->m_sopath      = sopath;
    this->m_soname      = basename((char*)sopath);
    this->m_fd          = fd;
    this->m_file_size   = file_stat.st_size;
    this->m_file_offset = 0;
    if (!this->_check_elf_header() || 
        !this->_read_segment_headers() ||
        !this->_read_section_headers() ||
        !this->_read_section_data()) {
        goto fail;
    }
    return true;
fail:
    close();
    return false;
}
void elf_reader::close() {
    if (m_fd != -1) {
        ::close(m_fd);
    }
    m_fd = -1;
}

elf_image* elf_reader::load() {
    assert(m_fd != -1);
    size_t min_aligment = _get_min_aligment((void*)m_phdr, m_phdr_num);
    if (min_aligment < PAGE_SIZE) {
        log_info("min_align(0x%lx), page_size(0x%lx), read_segments\n", min_aligment, (size_t)PAGE_SIZE);
        if (!_read_segments()) {
            return NULL;
        }
    } else {
        log_info("min_align(0x%lx), page_size(0x%lx), load_segments\n", min_aligment, (size_t)PAGE_SIZE);
        if (!_load_segments()) {
            return NULL;
        }
    }

    elf_image * image = NULL;
    if (this->get_elf_class() == ELFCLASS32) {
        image = new elf_image32(this);
    } else if (this->get_elf_class() == ELFCLASS64) {
        image = new elf_image64(this);
    }
    if (image) {
        if (image->load()) {
            return image;
        }
        delete image;
    }
    return NULL;
}

void* elf_reader::find_section_by_type(const uint32_t type) {
    void* target = NULL;
    assert(get_elf_class() == ELFCLASS32 || get_elf_class() == ELFCLASS64); 
    if (get_elf_class() == ELFCLASS32) {
        Elf32_Shdr* shdr = (Elf32_Shdr*)this->m_shdr;
        for(int i = 0; i < this->m_shdr_num; i += 1) {
            if(shdr[i].sh_type == type) {
                target = (void*)(shdr + i);
                break;
            }
        }
    } else if (get_elf_class() == ELFCLASS64) {
        Elf64_Shdr* shdr = (Elf64_Shdr*)this->m_shdr;
        for(int i = 0; i < this->m_shdr_num; i += 1) {
            if(shdr[i].sh_type == type) {
                target = (void*)(shdr + i);
                break;
            }
        }
    }
    return target;
}

void* elf_reader::find_section_by_name(const char *sname) {
    void* target = NULL;
    assert(get_elf_class() == ELFCLASS32 || get_elf_class() == ELFCLASS64); 
    if (!m_shstr) {
        return NULL;
    }
    if (get_elf_class() == ELFCLASS32) {
        Elf32_Shdr* shdr = (Elf32_Shdr*)this->m_shdr;
        for(int i = 0; i < this->m_shdr_num; i += 1) {
            const char *name = (const char *)(shdr[i].sh_name + this->m_shstr);
            if(name != NULL && !strncmp(name, sname, strlen(sname))) {
                target = (void*)(shdr + i);
                break;
            }
        }
    } else if (get_elf_class() == ELFCLASS64) {
        Elf64_Shdr* shdr = (Elf64_Shdr*)this->m_shdr;
        for(int i = 0; i < this->m_shdr_num; i += 1) {
            const char *name = (const char *)(shdr[i].sh_name + this->m_shstr);
            if(name != NULL && !strncmp(name, sname, strlen(sname))) {
                target = (void*)(shdr + i);
                break;
            }
        }
    }
    return target;
}

bool elf_reader::_check_elf_header() {
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
            log_error("unsupported elf type(0x%04x)\n", ehdr->e_type);
            return false;
        }
        if (ehdr->e_machine != EM_AARCH64 && ehdr->e_machine != EM_X86_64)  {
            log_error("unsupported class64 cpu arch\n");
            return false;
        }
        if (sizeof(Elf64_Phdr) != ehdr->e_phentsize) {
            log_error("bad phdr entry size: %d\n", ehdr->e_phentsize); 
            return false;
        }
        if (sizeof(Elf64_Shdr) != ehdr->e_shentsize) {
            log_error("bad shdr entry size: %d\n", ehdr->e_shentsize); 
            return false;
        }
    } else if (elf_class == ELFCLASS32) {
        Elf32_Ehdr *ehdr = &m_ehdr.ehdr32;
        if (ehdr->e_type != ET_DYN && ehdr->e_type != ET_EXEC) {
            log_error("unsupported elf type(0x%04x)\n", ehdr->e_type);
            return false;
        }
        if (ehdr->e_machine != EM_ARM && ehdr->e_machine != EM_386 && ehdr->e_machine != EM_486 )  {
            log_error("unsupported class32 cpu arch\n");
            return false;
        }
        if (sizeof(Elf32_Phdr) != ehdr->e_phentsize) {
            log_error("bad phdr entry size: %d\n", ehdr->e_phentsize); 
            return false;
        }
        if (sizeof(Elf32_Shdr) != ehdr->e_shentsize) {
            log_error("bad shdr entry size: %d\n", ehdr->e_shentsize); 
            return false;
        }
    } else {
        log_error("unsupported elf Class: %d\n", get_elf_class()); 
        return false;
    }
    return true;
}

bool elf_reader::_read_section_headers() {

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
    if (!_check_file_range(shdr_offset, size, 1)) {
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

    size_t shstr_size = 0;
    addr_t shstr_offset = 0;
    if (get_elf_class() == ELFCLASS64) {
        Elf64_Shdr* shstr_shdr = &((Elf64_Shdr*)this->m_shdr)[shstrndx];
        shstr_offset          = shstr_shdr->sh_offset;
        shstr_size            = shstr_shdr->sh_size;
    } else if (get_elf_class() == ELFCLASS32) {
        Elf32_Shdr* shstr_shdr = &((Elf32_Shdr*)this->m_shdr)[shstrndx];
        shstr_offset          = shstr_shdr->sh_offset;
        shstr_size             = shstr_shdr->sh_size;
    } else {
        log_error("unsupported ELF Class: %d\n", get_elf_class()); 
        return false;
    }

    if (!this->_check_file_range(shstr_offset, shstr_size, 1)) {
       log_error("\"%s\" has invalid shdr offset/size: %zu/%zu\n",
                  this->get_sopath(),
                  (size_t)shstr_offset,
                  (size_t)shstr_size);
       return false;
    }
    if (!this->m_shstr_fragment.map(this->m_fd, 0, shstr_offset, shstr_size)) {
        log_error("\"%s\" shstr mmap failed: %s\n", this->get_sopath(), strerror(errno));
        return false;
    }
    this->m_shstr = (const char *)this->m_shstr_fragment.data();
    this->m_shstr_size = shstr_size;
    log_info("read section header: shdr(0x%p), shdr_num(%zu)\n", this->m_shdr, this->m_shdr_num);
    return true;
}

bool elf_reader::_read_segment_headers() {
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
        log_error("unsupported elf class: %d\n", get_elf_class()); 
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
    if (!_check_file_range(phdr_offset, size, 4)) {
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

bool elf_reader::_read_section_data(void) {
    if (get_elf_class() == ELFCLASS64) {
        Elf64_Shdr * symstr_shdr = NULL;
        Elf64_Shdr * symtab_shdr = NULL;
        Elf64_Shdr * shdr = (Elf64_Shdr*)this->m_shdr;
        for (size_t i = 0; i < this->m_shdr_num; ++i) {
            const char * sh_name = &this->m_shstr[shdr[i].sh_name];
            if (shdr[i].sh_type == SHT_STRTAB) {
                if (strncmp(sh_name, ".strtab", 7) == 0) {
                    symstr_shdr = &shdr[i];
                }
            } else if (shdr[i].sh_type == SHT_SYMTAB) {
                if (strncmp(sh_name, ".symtab", 7) == 0) {
                    symtab_shdr = &shdr[i];
                }
            }
        }
        if (symtab_shdr && 
            _check_file_range(symtab_shdr->sh_offset, symtab_shdr->sh_size, 4)) {
            if (!this->m_symtab_fragment.map(this->m_fd, 0, symtab_shdr->sh_offset, symtab_shdr->sh_size)) {
                log_warn("symtab map fail, %s\n", strerror(errno));
            }
            this->m_symtab = (void *)this->m_symtab_fragment.data();
            this->m_symtab_size = symtab_shdr->sh_size;
        }
        if (symstr_shdr && 
            _check_file_range(symstr_shdr->sh_offset, symstr_shdr->sh_size, 1)) {
            if (!this->m_symstr_fragment.map(this->m_fd, 0, symstr_shdr->sh_offset, symstr_shdr->sh_size)) {
                log_warn("symstr map fail, %s\n", strerror(errno));
            }
            this->m_symstr = (const char *)this->m_symstr_fragment.data();
            this->m_symstr_size = symstr_shdr->sh_size; 
        }
        return true;
    } else if (get_elf_class() == ELFCLASS32) {
        Elf32_Shdr * symstr_shdr = NULL;
        Elf32_Shdr * symtab_shdr = NULL;
        Elf32_Shdr * shdr = (Elf32_Shdr*)this->m_shdr;
        for (size_t i = 0; i < this->m_shdr_num; ++i) {
            const char * sh_name = &this->m_shstr[shdr[i].sh_name];
            if (shdr[i].sh_type == SHT_STRTAB) {
                if (strncmp(sh_name, ".strtab", 7) == 0) {
                    symstr_shdr = &shdr[i];
                } 
            } else if (shdr[i].sh_type == SHT_SYMTAB) {
                if (strncmp(sh_name, ".symtab", 7) == 0) {
                    symtab_shdr = &shdr[i];
                }
            }
        }
        if (symtab_shdr && 
            _check_file_range(symtab_shdr->sh_offset, symtab_shdr->sh_size, 4)) {
            if (!this->m_symtab_fragment.map(this->m_fd, 0, symtab_shdr->sh_offset, symtab_shdr->sh_size)) {
                log_warn("symtab map fail, %s\n", strerror(errno));
            }
            this->m_symtab = (void *)this->m_symtab_fragment.data();
            this->m_symtab_size = symtab_shdr->sh_size;
        }
        if (symstr_shdr && 
            _check_file_range(symstr_shdr->sh_offset, symstr_shdr->sh_size, 1)) {
            if (!this->m_symstr_fragment.map(this->m_fd, 0, symstr_shdr->sh_offset, symstr_shdr->sh_size)) {
                log_warn("strtab map fail, %s\n", strerror(errno));
            }
            this->m_symstr = (const char *)this->m_symstr_fragment.data();
            this->m_symstr_size = symstr_shdr->sh_size; 
        }
        return true;
    }
    log_error("unsupported ELF Class: %d", get_elf_class()); 
    return false;
}

bool elf_reader::_read_segments(void) {
    addr_t p_min_addr = (addr_t)NULL;
    addr_t p_max_addr = (addr_t)NULL;

    size_t load_size = _get_load_size((void*)m_phdr, m_phdr_num, &p_min_addr, &p_max_addr);
    assert(load_size > 0);
    
    void* mmap_ptr = mmap(nullptr, load_size,  PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mmap_ptr == MAP_FAILED) {
        log_error("reserve address space fail, load(0x%zu), error(%s)\n", load_size, strerror(errno));
        return false;
    }
    m_load_bias = (addr_t)mmap_ptr - p_min_addr;
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
        log_info("read segment: i(%d), addr(%p), offset(%p), filesz(%lx), nread(%lx)\n", 
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

bool elf_reader::_load_segments(void) {
    addr_t p_min_addr = (addr_t)NULL;
    addr_t p_max_addr = (addr_t)NULL;
      
    size_t load_size = _get_load_size((void*)m_phdr, m_phdr_num, &p_min_addr, &p_max_addr);
    assert(load_size > 0);

    void* mmap_ptr = mmap(nullptr, load_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
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

        void* seg_addr = mmap((void*)seg_page_start,
                                file_length,
                                PROT_WRITE | PROT_READ,
                                MAP_FIXED | MAP_PRIVATE,
                                this->m_fd,
                                this->m_file_offset + file_page_start);
        if (seg_addr == MAP_FAILED) {
            log_error("couldn't map \"%s\" segment %d: %s\n", m_soname.c_str(), i, strerror(errno));
            return false;
        }
    }
    return true;
}

size_t elf_reader::_get_min_aligment(void* phdr, size_t phdr_num) {
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

size_t elf_reader::_get_load_size(void* phdr, size_t phdr_num, addr_t* out_min_vaddr, addr_t* out_max_vaddr) {
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

bool elf_reader::_check_file_range(off_t offset, size_t size, size_t alignment) {
    off_t range_start;
    off_t range_end;
    return offset > 0 &&
        elf_safe_add(&range_start, 0, offset) &&
        elf_safe_add(&range_end, range_start, size) &&
        (range_start < m_file_size) &&
        (range_end <= m_file_size) &&
        ((offset % alignment) == 0);
}