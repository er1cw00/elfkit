cmake_minimum_required(VERSION 3.18.1)

set(PROJECT "elfkit")

set(BIN_NAME "${PROJECT}")
set(LIB_NAME "${PROJECT}-dev")

project(${PROJECT})

aux_source_directory(src  SOURCE_FILES)

file(GLOB HEADER_FILES src/*.h)

message(status "HEADER_FILES: ${HEADER_FILES}")

set(CMAKE_CXX_FLAGS    "${CMAKE_CXX_FLAGS}    -fno-omit-frame-pointer -Wno-null-conversion -Wno-null-arithmetic")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer ")

add_library(${LIB_NAME} STATIC ${SOURCE_FILES})

set_target_properties(${LIB_NAME} PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")

add_executable(${BIN_NAME} example/main.cc)

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