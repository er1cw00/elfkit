cmake_minimum_required(VERSION 3.18.1)

enable_testing()

add_executable(test_types test_types.cc)
target_link_libraries(test_types elfkit)
add_test(types test_types)

add_executable(test_hashtab test_hashtab.cc)
target_link_libraries(test_hashtab elfkit)
add_test(hashtab test_hashtab)

add_executable(test_strtab test_strtab.cc)
target_link_libraries(test_strtab elfkit)
add_test(strtab test_strtab)

add_executable(test_symtab test_symtab.cc)
target_link_libraries(test_symtab elfkit)
add_test(symtab test_symtab)

add_executable(test_reloctab test_reloctab.cc)
target_link_libraries(test_reloctab elfkit)
add_test(reloctab test_reloctab)