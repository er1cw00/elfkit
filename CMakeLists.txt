cmake_minimum_required(VERSION 3.18.1)

set(PROJECT "elfkit")
set(BIN_NAME "${PROJECT}")
set(LIB_NAME "${PROJECT}-dev")


project(${PROJECT})


set(SOURCES src/elf_common.cc
            src/elf_mapped.cc
            src/elf_reader.cc
            src/elf_image.cc
            src/elf_image32.cc
            src/elf_image64.cc)

set(HEADERS src/elf.h
            src/elf_common.h
            src/elf_image.h
            src/elf_image32.h
            src/elf_image64.h)

set(CMAKE_CXX_FLAGS    "${CMAKE_CXX_FLAGS}           -fno-omit-frame-pointer -Wno-null-conversion -Wno-null-arithmetic")
set(CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fno-omit-frame-pointer ")


add_library(${LIB_NAME} STATIC ${SOURCES})

#include_directories(./inc)
set_target_properties(${LIB_NAME} PROPERTIES PUBLIC_HEADER "${HEADERS}")


add_executable(${BIN_NAME} src/main.cc)
target_link_libraries(${BIN_NAME} ${LIB_NAME})