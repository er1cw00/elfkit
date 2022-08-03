cmake_minimum_required(VERSION 3.18.1)

set(PROJECT "elfkit")

set(BIN_NAME "${PROJECT}")
set(LIB_NAME "${PROJECT}-dev")


project(${PROJECT})


aux_source_directory(src  SOURCES)

set(HEADERS src/string_list.h
            src/elf.h
            src/elf_log.h
            src/elf_common.h
            src/elf_mapped.h
            src/elf_reader.h
            src/elf_image.h
            src/elf_reloc.h
            src/elf_symbol.h
            src/elf_section.h
            src/elf_segment.h
            src/elf_func_array.h
            src/elf_reloc_tab.h
            src/elf_hash_tab.h
            src/elf_symbol_tab.h
            src/elf_string_tab.h)

set(CMAKE_CXX_FLAGS    "${CMAKE_CXX_FLAGS}           -fno-omit-frame-pointer -Wno-null-conversion -Wno-null-arithmetic")
set(CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fno-omit-frame-pointer ")


add_library(${LIB_NAME} STATIC ${SOURCES})


set_target_properties(${LIB_NAME} PROPERTIES PUBLIC_HEADER "${HEADERS}")


add_executable(${BIN_NAME} src/main.cc)

target_link_libraries(${BIN_NAME} ${LIB_NAME})

install(
    TARGETS
        ${PROJECT}-dev
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        COMPONENT lib
    PUBLIC_HEADER
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/elfkit
        COMPONENT dev
)