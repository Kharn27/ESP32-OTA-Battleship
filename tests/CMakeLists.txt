cmake_minimum_required(VERSION 3.16)
project(warship_host_tests C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(warship_core STATIC ../components/warship_core/warship_core.c)
target_include_directories(warship_core PUBLIC ../components/warship_core/include)
target_compile_options(warship_core PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(warship_core_tests test_warship_core.c)
target_link_libraries(warship_core_tests PRIVATE warship_core)
target_compile_options(warship_core_tests PRIVATE -Wall -Wextra -Werror -pedantic)

if(WARSHIP_SANITIZERS)
    foreach(target warship_core warship_core_tests)
        target_compile_options(${target} PRIVATE -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endforeach()
endif()

enable_testing()
add_test(NAME warship_core_contract COMMAND warship_core_tests)
