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

add_library(warship_fleet STATIC ../components/warship_fleet/warship_fleet.c)
target_include_directories(warship_fleet PUBLIC ../components/warship_fleet/include)
target_compile_options(warship_fleet PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(warship_fleet_tests test_warship_fleet.c)
target_link_libraries(warship_fleet_tests PRIVATE warship_fleet)
target_compile_options(warship_fleet_tests PRIVATE -Wall -Wextra -Werror -pedantic)

add_library(warship_battle STATIC ../components/warship_battle/warship_battle.c)
target_include_directories(warship_battle PUBLIC ../components/warship_battle/include)
target_link_libraries(warship_battle PUBLIC warship_fleet)
target_compile_options(warship_battle PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(warship_battle_tests test_warship_battle.c)
target_link_libraries(warship_battle_tests PRIVATE warship_battle)
target_compile_options(warship_battle_tests PRIVATE -Wall -Wextra -Werror -pedantic)

if(WARSHIP_SANITIZERS)
    foreach(target warship_core warship_core_tests t02_touch_map t02_touch_map_tests warship_fleet warship_fleet_tests warship_battle warship_battle_tests)
        target_compile_options(${target} PRIVATE -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endforeach()
endif()

enable_testing()
add_test(NAME warship_core_contract COMMAND warship_core_tests)
add_test(NAME t02_touch_map_boundaries COMMAND t02_touch_map_tests)
add_test(NAME warship_fleet_contract COMMAND warship_fleet_tests)
add_test(NAME warship_battle_contract COMMAND warship_battle_tests)
