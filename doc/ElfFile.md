# ELF格式加载


## 1. ELF 文件头
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
| e_ident    | <p>第0-2字节是"ELF"的ascii码</p><p>第4字节是ELF_CLASS,表示该文件为32位或者64位</p><p>第5字节是字节序</p>|
| e_phoff    | phdr 偏移地址 |
| e_shoff    | shdr 偏移地址 |
| e_phnum    | phdr 数量 |
| e_shnum    | shdr 数量 |
| e_shstrndx | shdr 名字的字符串表shdr索引 |



## 2. Section与Segment
 
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
|sh_name| 节名字，在shstr表中的索引值|
|sh_type| 节类型 |
|sh_flags|  |
|sh_addr|   |
|sh_size|  |
|sh_link| |
|sh_info| |
|sh_addralign| |
|sh_entsize| |

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
|p_type| 段类型|
|p_flags| 读/写/执行权限标志|
|p_offset| 本段数据在文件中的偏移地址|
|p_vaddr| 本段数据在需要加载到内存中的虚拟地址|
|p_paddr| 本段数据在需要加载到内存中的物理地址,一般不使用|
|p_filesz| 本段数据在文件中的大小|
|p_memsz| 本段数据加载到内存后的大小|
|p_align| 最小字节对齐|


PT_NULL:    解析到该类型的phdr后，结束解析

PT_LOAD:    该类型phdr指向的segment，直接加载到内存中

PT_DYNAMIC: 动态段是ELF文件的动态链接信息，由该段的数据链接，重定位符号

## 2. Dynamic段与加载
## 3. 符号表与字符串表
## 4. 重定位表
