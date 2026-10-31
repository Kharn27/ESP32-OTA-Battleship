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

add_library(t02_touch_map STATIC ../components/t02_touch_map/t02_touch_map.c)
target_include_directories(t02_touch_map PUBLIC ../components/t02_touch_map/include)
target_compile_options(t02_touch_map PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(t02_touch_map_tests test_t02_touch_map.c)
target_link_libraries(t02_touch_map_tests PRIVATE t02_touch_map)
target_compile_options(t02_touch_map_tests PRIVATE -Wall -Wextra -Werror -pedantic)

if(WARSHIP_SANITIZERS)
    foreach(target warship_core warship_core_tests t02_touch_map t02_touch_map_tests)
        target_compile_options(${target} PRIVATE -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endforeach()
endif()

enable_testing()
add_test(NAME warship_core_contract COMMAND warship_core_tests)
add_test(NAME t02_touch_map_boundaries COMMAND t02_touch_map_tests)
