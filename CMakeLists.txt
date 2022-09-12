cmake_minimum_required(VERSION 3.18.1)

set(PROJECT "elfkit")

set(BIN_NAME "${PROJECT}")
set(LIB_NAME "${PROJECT}-dev")

project(${PROJECT})

message(status " CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")

aux_source_directory(src         SOURCE_FILES)
aux_source_directory(src/common  SOURCE_FILES)
aux_source_directory(src/reader  SOURCE_FILES)
aux_source_directory(src/image   SOURCE_FILES)
aux_source_directory(src/model   SOURCE_FILES)

#file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src "*.cc")
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src "*.h")

message(status " SOURCE_FILES : ${SOURCE_FILES}")
message(status " HEADER_FILES : ${HEADER_FILES}")

set(CMAKE_CXX_FLAGS    "${CMAKE_CXX_FLAGS}    -fno-omit-frame-pointer -Wno-null-conversion -Wno-null-arithmetic")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer ")

add_library(${LIB_NAME} STATIC ${SOURCE_FILES})

set_target_properties(${LIB_NAME} PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")
target_include_directories(${LIB_NAME} PUBLIC src)


add_executable(${BIN_NAME} example/main.cc)

target_include_directories(${BIN_NAME} PRIVATE src)

target_link_libraries(${BIN_NAME} ${LIB_NAME})

install(
    TARGETS
        ${PROJECT}-dev
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        COMPONENT lib
)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/src" # source directory
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/elfkit # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
)