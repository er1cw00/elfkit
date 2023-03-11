# ELF File and Loader


## 1. ELF File Header
```` c++
#define EI_NIDENT     16

typedef struct elf64_hdr {
  unsigned char e_ident[EI_NIDENT];
  Elf64_Half e_type;
  Elf64_Half e_machine;
  Elf64_Word e_version;
  Elf64_Addr e_entry;
  Elf64_Off e_phoff;
  Elf64_Off e_shoff;
  Elf64_Word e_flags;
  Elf64_Half e_ehsize;
  Elf64_Half e_phentsize;
  Elf64_Half e_phnum;
  Elf64_Half e_shentsize;
  Elf64_Half e_shnum;
  Elf64_Half e_shstrndx;
} Elf64_Ehdr;

````

| 字段 | 说明 |
| :- | :--- |
| e_ident    | <p>bytes 0-2 is ascii of "ELF"</p><p>byte 4th is ELF_CLASS,  indicates this ELF file is 32 or 64 bit</p><p>byte 5th is byte order</p>|
| e_phoff    | phdr offset  |
| e_shoff    | shdr offset  |
| e_phnum    | nums of phdr |
| e_shnum    | nums of shdr |
| e_shstrndx | the shdr index that store string table of shdr name|



## 2. Section & Segment

 
### 2.1 Section

````c++
typedef struct elf64_shdr {
    Elf64_Word sh_name;
    Elf64_Word sh_type;
    Elf64_Xword sh_flags;
    Elf64_Addr sh_addr;
    Elf64_Off sh_offset;
    Elf64_Xword sh_size;
    Elf64_Word sh_link;
    Elf64_Word sh_info;
    Elf64_Xword sh_addralign;
    Elf64_Xword sh_entsize;
} Elf64_Shdr;
````
| 字段 | 说明 |
| :- | :--- |
|sh_name     | name of shdr，index in shstr section|
|sh_type     | typef of shdr |
|sh_flags    |  |
|sh_addr     | addr of section detail |
|sh_size     | size of section detail |
|sh_link     | |
|sh_info     | |
|sh_addralign| aligment addr of section|
|sh_entsize  | size of section header|

### 2.2 Segment

````c++
typedef struct elf64_phdr {
    Elf64_Word p_type;
    Elf64_Word p_flags;
    Elf64_Off p_offset;
    Elf64_Addr p_vaddr;
    Elf64_Addr p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
  } Elf64_Phdr;

````
| 字段 | 说明 |
| :- | :--- |
|p_type  | type of segment |
|p_flags | 读/写/执行权限标志|
|p_offset| offset of segment detail in ELF file|
|p_vaddr | virtual addr segment detail should load |
|p_paddr | physics addr segment detail should load (useless)|
|p_filesz| size of segment detail in ELF file|
|p_memsz | size of segment detail  loaded in memory|
|p_align | 最小字节对齐|


PT_NULL:    解析到该类型的phdr后，结束解析

PT_LOAD:    该类型phdr指向的segment，直接加载到内存中

PT_DYNAMIC: 动态段是ELF文件的动态链接信息，由该段的数据链接，重定位符号

## 2. Dynamic segment 
## 3. Symbol & String table
## 4. Relocate 
