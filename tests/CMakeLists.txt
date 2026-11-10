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

add_library(warship_captain STATIC ../components/warship_captain/warship_captain.c)
target_include_directories(warship_captain PUBLIC ../components/warship_captain/include)
target_link_libraries(warship_captain PUBLIC warship_battle)
target_compile_options(warship_captain PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(warship_captain_tests test_warship_captain.c)
target_link_libraries(warship_captain_tests PRIVATE warship_captain)
target_compile_options(warship_captain_tests PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(warship_captain_tracks_tests test_warship_captain_tracks.c)
target_link_libraries(warship_captain_tracks_tests PRIVATE warship_captain)
target_compile_options(warship_captain_tracks_tests PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(warship_captain_geometry_tests test_warship_captain_geometry.c)
target_link_libraries(warship_captain_geometry_tests PRIVATE warship_captain)
target_compile_options(warship_captain_geometry_tests PRIVATE -Wall -Wextra -Werror -pedantic)

add_library(warship_solo STATIC ../components/warship_solo/warship_solo.c)
target_include_directories(warship_solo PUBLIC ../components/warship_solo/include)
target_link_libraries(warship_solo PUBLIC warship_captain)
target_compile_options(warship_solo PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(warship_solo_tests test_warship_solo.c)
target_link_libraries(warship_solo_tests PRIVATE warship_solo)
target_compile_options(warship_solo_tests PRIVATE -Wall -Wextra -Werror -pedantic)

add_library(warship_lifecycle STATIC ../components/warship_lifecycle/warship_lifecycle.c)
target_include_directories(warship_lifecycle PUBLIC ../components/warship_lifecycle/include)
target_link_libraries(warship_lifecycle PUBLIC warship_solo)
target_compile_options(warship_lifecycle PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(warship_lifecycle_tests test_warship_lifecycle.c)
target_link_libraries(warship_lifecycle_tests PRIVATE warship_lifecycle)
target_compile_options(warship_lifecycle_tests PRIVATE -Wall -Wextra -Werror -pedantic)

add_library(warship_palmares STATIC ../components/warship_palmares/warship_palmares.c)
target_include_directories(warship_palmares PUBLIC ../components/warship_palmares/include)
target_link_libraries(warship_palmares PUBLIC warship_lifecycle)
target_compile_options(warship_palmares PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(warship_palmares_tests test_warship_palmares.c)
target_link_libraries(warship_palmares_tests PRIVATE warship_palmares)
target_compile_options(warship_palmares_tests PRIVATE -Wall -Wextra -Werror -pedantic)

add_library(warship_event_bridge STATIC
            ../components/warship_event_bridge/warship_event_bridge.c)
target_include_directories(warship_event_bridge PUBLIC
                           ../components/warship_event_bridge/include)
target_link_libraries(warship_event_bridge PUBLIC warship_palmares)
target_compile_options(warship_event_bridge PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(warship_event_bridge_tests test_warship_event_bridge.c)
target_link_libraries(warship_event_bridge_tests PRIVATE warship_event_bridge)
target_compile_options(warship_event_bridge_tests PRIVATE
                       -Wall -Wextra -Werror -pedantic)

# Compile the actual ESP-IDF NVS adapter against host-test stubs.
# No ESP-IDF or hardware required; exercises set_blob / commit failures.
add_library(warship_palmares_nvs_host STATIC
            ../components/warship_palmares_nvs/warship_palmares_nvs.c)
target_include_directories(warship_palmares_nvs_host PRIVATE nvs_fake)
target_include_directories(warship_palmares_nvs_host PUBLIC
                           ../components/warship_palmares_nvs/include)
target_link_libraries(warship_palmares_nvs_host PUBLIC warship_palmares)
target_compile_options(warship_palmares_nvs_host PRIVATE
                       -Wall -Wextra -Werror -pedantic)

add_executable(warship_palmares_nvs_host_tests test_warship_palmares_nvs.c)
target_include_directories(warship_palmares_nvs_host_tests PRIVATE nvs_fake)
target_link_libraries(warship_palmares_nvs_host_tests PRIVATE
                      warship_palmares_nvs_host)
target_compile_options(warship_palmares_nvs_host_tests PRIVATE
                       -Wall -Wextra -Werror -pedantic)

add_library(warship_deploy_ui STATIC
            ../components/warship_deploy_ui/warship_deploy_ui.c
            ../components/warship_deploy_ui/warship_deploy_render.c)
target_include_directories(warship_deploy_ui PUBLIC
                           ../components/warship_deploy_ui/include)
target_link_libraries(warship_deploy_ui PUBLIC warship_lifecycle)
target_compile_options(warship_deploy_ui PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(warship_deploy_ui_tests test_warship_deploy_ui.c)
target_link_libraries(warship_deploy_ui_tests PRIVATE warship_deploy_ui)
target_compile_options(warship_deploy_ui_tests PRIVATE -Wall -Wextra -Werror -pedantic)

# D09: real PO XPT2046 medians through the calibrated mapping and the
# unmodified B01 dispatcher, tested together with existing D09 host cases.
add_executable(d09_touch_mapping_tests test_d09_touch_mapping.c)
target_link_libraries(d09_touch_mapping_tests PRIVATE
                      t02_touch_map warship_deploy_ui)
target_compile_options(d09_touch_mapping_tests PRIVATE
                       -Wall -Wextra -Werror -pedantic)

add_library(warship_combat_ui STATIC
            ../components/warship_combat_ui/warship_combat_ui.c
            ../components/warship_combat_ui/warship_combat_render.c)
target_include_directories(warship_combat_ui PUBLIC
                           ../components/warship_combat_ui/include)
target_link_libraries(warship_combat_ui PUBLIC warship_lifecycle)
target_compile_options(warship_combat_ui PRIVATE
                       -Wall -Wextra -Werror -pedantic)

add_executable(warship_combat_ui_tests test_warship_combat_ui.c)
target_link_libraries(warship_combat_ui_tests PRIVATE warship_combat_ui)
target_compile_options(warship_combat_ui_tests PRIVATE
                       -Wall -Wextra -Werror -pedantic)

# Host-compile the actual ESP-IDF event-ID reservation adapter with NVS stubs.
add_library(warship_event_id_nvs_host STATIC
            ../components/warship_event_id_nvs/warship_event_id_nvs.c)
target_include_directories(warship_event_id_nvs_host PRIVATE nvs_fake)
target_include_directories(warship_event_id_nvs_host PUBLIC
                           ../components/warship_event_id_nvs/include)
target_link_libraries(warship_event_id_nvs_host PUBLIC warship_event_bridge)
target_compile_options(warship_event_id_nvs_host PRIVATE
                       -Wall -Wextra -Werror -pedantic)

add_executable(warship_event_id_nvs_tests test_warship_event_id_nvs.c)
target_include_directories(warship_event_id_nvs_tests PRIVATE nvs_fake)
target_link_libraries(warship_event_id_nvs_tests PRIVATE
                      warship_event_id_nvs_host)
target_compile_options(warship_event_id_nvs_tests PRIVATE
                       -Wall -Wextra -Werror -pedantic)

# D12: real D07 lifecycle + D10 tactile/technical UI + D11 ledger.
add_library(warship_journey STATIC
            ../components/warship_journey/warship_journey.c)
target_include_directories(warship_journey PUBLIC
                           ../components/warship_journey/include)
target_link_libraries(warship_journey PUBLIC
                      warship_combat_ui warship_event_bridge)
target_compile_options(warship_journey PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(warship_journey_tests test_warship_journey.c)
target_link_libraries(warship_journey_tests PRIVATE
                      warship_journey warship_deploy_ui)
target_compile_options(warship_journey_tests PRIVATE
                       -Wall -Wextra -Werror -pedantic)

# D13: real D07 permission + D10 raster/touch + D12 result journey.
add_library(warship_inspection STATIC
            ../components/warship_inspection/warship_inspection.c)
target_include_directories(warship_inspection PUBLIC
                           ../components/warship_inspection/include)
target_link_libraries(warship_inspection PUBLIC warship_combat_ui)
target_compile_options(warship_inspection PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(warship_inspection_tests test_warship_inspection.c)
target_link_libraries(warship_inspection_tests PRIVATE
                      warship_inspection warship_journey warship_palmares)
target_compile_options(warship_inspection_tests PRIVATE
                       -Wall -Wextra -Werror -pedantic)

if(WARSHIP_SANITIZERS)
    foreach(target warship_core warship_core_tests t02_touch_map t02_touch_map_tests warship_fleet warship_fleet_tests warship_battle warship_battle_tests warship_captain warship_captain_tests warship_captain_tracks_tests warship_captain_geometry_tests warship_solo warship_solo_tests warship_lifecycle warship_lifecycle_tests warship_palmares warship_palmares_tests warship_event_bridge warship_event_bridge_tests warship_event_id_nvs_host warship_event_id_nvs_tests warship_palmares_nvs_host warship_palmares_nvs_host_tests warship_deploy_ui warship_deploy_ui_tests d09_touch_mapping_tests warship_combat_ui warship_combat_ui_tests warship_journey warship_journey_tests warship_inspection warship_inspection_tests)
        target_compile_options(${target} PRIVATE -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endforeach()
endif()

enable_testing()
add_test(NAME warship_core_contract COMMAND warship_core_tests)
add_test(NAME t02_touch_map_boundaries COMMAND t02_touch_map_tests)
add_test(NAME warship_fleet_contract COMMAND warship_fleet_tests)
add_test(NAME warship_battle_contract COMMAND warship_battle_tests)
add_test(NAME warship_captain_foundation COMMAND warship_captain_tests)
add_test(NAME warship_captain_tracks COMMAND warship_captain_tracks_tests)
add_test(NAME warship_captain_geometry COMMAND warship_captain_geometry_tests)
add_test(NAME warship_solo_session COMMAND warship_solo_tests)
add_test(NAME warship_lifecycle_session COMMAND warship_lifecycle_tests)
add_test(NAME warship_palmares_persistence COMMAND warship_palmares_tests)
add_test(NAME warship_palmares_nvs_adapter COMMAND warship_palmares_nvs_host_tests)
add_test(NAME warship_deploy_ui_technical COMMAND warship_deploy_ui_tests)

add_test(NAME d09_touch_calibrated_commands COMMAND d09_touch_mapping_tests)

add_test(NAME d10_combat_ui_technical COMMAND warship_combat_ui_tests)

add_test(NAME d11_event_bridge_and_d07 COMMAND warship_event_bridge_tests)
add_test(NAME d11_nvs_id_reservation COMMAND warship_event_id_nvs_tests)

add_test(NAME d12_technical_journey COMMAND warship_journey_tests)

add_test(NAME d13_post_result_inspection COMMAND warship_inspection_tests)


# T03: build two lossless Flash representations from the *accepted G03 PNG*.
# All generated C stays in the build tree, independently regenerated in CI.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(T03_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../design/ecrans/deploiement/g03_deploiement_320x240.png")
set(T03_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../tools/t03_export.py")
set(T03_GENERATED "${CMAKE_CURRENT_BINARY_DIR}/t03_g03_data.c")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
             "${T03_SOURCE}" "${T03_SCRIPT}")
execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${T03_SCRIPT}"
          --input "${T03_SOURCE}" --output "${T03_GENERATED}"
  RESULT_VARIABLE T03_EXPORT_EXIT OUTPUT_VARIABLE T03_EXPORT_METRICS
  ERROR_VARIABLE T03_EXPORT_ERROR)
if(NOT T03_EXPORT_EXIT EQUAL 0)
  message(FATAL_ERROR "T03 G03 export failed: ${T03_EXPORT_ERROR}")
endif()
message(STATUS "T03 G03 exact export ${T03_EXPORT_METRICS}")
add_executable(t03_asset_tests test_t03_asset.c
               ../components/t03_decode/t03_decode.c "${T03_GENERATED}")
target_include_directories(t03_asset_tests PRIVATE ../components/t03_decode/include)
target_compile_options(t03_asset_tests PRIVATE -Wall -Wextra -Werror -pedantic)
if(WARSHIP_SANITIZERS)
  target_compile_options(t03_asset_tests PRIVATE
                         -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined)
  target_link_options(t03_asset_tests PRIVATE -fsanitize=address,undefined)
endif()
add_test(NAME t03_g03_exact_raw_vs_rle COMMAND t03_asset_tests)


# D14: approved G03 opaque background + transparent G02 ship sprites.
# The same generator is run in host/IDF configurations and outputs only
# into the build tree; no design-source files are modified.
set(D14_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../design/ecrans/deploiement/g03_deploiement_320x240.svg")
set(D14_PNG "${CMAKE_CURRENT_SOURCE_DIR}/../design/ecrans/deploiement/g03_deploiement_320x240.png")
set(D14_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../tools/d14_export.py")
set(D14_GENERATED "${CMAKE_CURRENT_BINARY_DIR}/d14_g03_data.c")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
             "${D14_SOURCE}" "${D14_PNG}" "${D14_SCRIPT}")
execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${D14_SCRIPT}"
          --svg "${D14_SOURCE}" --png "${D14_PNG}"
          --output "${D14_GENERATED}"
  RESULT_VARIABLE D14_EXPORT_EXIT OUTPUT_VARIABLE D14_METRICS
  ERROR_VARIABLE D14_ERROR)
if(NOT D14_EXPORT_EXIT EQUAL 0)
  message(FATAL_ERROR "D14 asset extraction failed: ${D14_ERROR}")
endif()
message(STATUS "D14 G03 art/Flash ${D14_METRICS}")
add_executable(d14_g03_tests test_d14_g03.c
               ../components/d14_g03/d14_g03.c
               ../components/t03_decode/t03_decode.c
               "${D14_GENERATED}")
target_include_directories(d14_g03_tests PRIVATE
       ../components/d14_g03/include ../components/t03_decode/include)
target_link_libraries(d14_g03_tests PRIVATE warship_deploy_ui)
target_compile_options(d14_g03_tests PRIVATE -Wall -Wextra -Werror -pedantic)
if(WARSHIP_SANITIZERS)
    target_compile_options(d14_g03_tests PRIVATE
                          -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined)
    target_link_options(d14_g03_tests PRIVATE -fsanitize=address,undefined)
endif()
add_test(NAME d14_g03_deploy_runtime COMMAND d14_g03_tests)


# D14 PO correction: compile ACTUAL LEDC implementation with fake GPIO/LEDC
# instead of merely testing numerical percent mapping in a surrogate.
# Each case runs in a fresh process to reset the controller's static state.
add_executable(d14_backlight_tests
               test_d14_backlight_pwm.c
               ../components/warship_lcd/warship_lcd_light.c)
target_include_directories(d14_backlight_tests PRIVATE
                           ledc_fake ../components/warship_lcd)
target_compile_definitions(d14_backlight_tests PRIVATE
                           WARSHIP_LCD_BACKLIGHT_PERCENT=40)
target_compile_options(d14_backlight_tests PRIVATE
                       -Wall -Wextra -Werror -pedantic)
if(WARSHIP_SANITIZERS)
    target_compile_options(d14_backlight_tests PRIVATE
                           -O1 -g -fno-omit-frame-pointer
                           -fsanitize=address,undefined)
    target_link_options(d14_backlight_tests PRIVATE -fsanitize=address,undefined)
endif()
foreach(mode nominal init_timer_fail init_channel_fail stop_fail
             set_duty_fail update_fail frequency_fail)
    add_test(NAME d14_ledc_${mode}
             COMMAND d14_backlight_tests ${mode})
endforeach()


# D14 targeted color diagnosis, purely C99 and independent of ESP-IDF:
# exact wire-order swatches, 5 isolated profiles and known calibrated gamma.
add_executable(d14_color_diag_tests
               test_d14_color_diag.c
               ../components/d14_color_diag/d14_color_diag.c)
target_include_directories(d14_color_diag_tests PRIVATE
                           ../components/d14_color_diag/include)
target_compile_options(d14_color_diag_tests PRIVATE -Wall -Wextra -Werror -pedantic)
if(WARSHIP_SANITIZERS)
    target_compile_options(d14_color_diag_tests PRIVATE
                           -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined)
    target_link_options(d14_color_diag_tests PRIVATE -fsanitize=address,undefined)
endif()
add_test(NAME d14_color_profiles_and_rgb565_be COMMAND d14_color_diag_tests)


# D14 corrective color probe: the source is exactly the PO-approved G01
# PNG from PR#90 on main, SHA-256 pinned in generator, not old G01 PR#8.
# Golden is RGB565BE quantized directly from THAT PNG, no G03/SVG involved.
set(D14_G01_PNG "${CMAKE_CURRENT_SOURCE_DIR}/../design/ecrans/accueil/accueil_cyd_warship_320x240.png")
set(D14_G01_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../tools/d14_color_g01_export.py")
set(D14_G01_C "${CMAKE_CURRENT_BINARY_DIR}/d14_color_g01_data.c")
set(D14_G01_RAW "${CMAKE_CURRENT_BINARY_DIR}/d14_color_g01_ref.rgb565be")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
             "${D14_G01_PNG}" "${D14_G01_SCRIPT}")
execute_process(
    COMMAND "${Python3_EXECUTABLE}" "${D14_G01_SCRIPT}"
            --input "${D14_G01_PNG}" --output "${D14_G01_C}"
            --raw-reference "${D14_G01_RAW}"
    RESULT_VARIABLE D14_G01_STATUS OUTPUT_VARIABLE D14_G01_METRICS
    ERROR_VARIABLE D14_G01_ERROR)
if(NOT D14_G01_STATUS EQUAL 0)
    message(FATAL_ERROR "D14 G01 PR90 PNG hash/encode error: ${D14_G01_ERROR}")
endif()
message(STATUS "D14 G01 PR90 test source: ${D14_G01_METRICS}")
add_executable(d14_g01_pr90_tests test_d14_color_g01.c
               ../components/t03_decode/t03_decode.c "${D14_G01_C}")
target_include_directories(d14_g01_pr90_tests PRIVATE
                           ../components/t03_decode/include)
target_compile_options(d14_g01_pr90_tests PRIVATE
                       -Wall -Wextra -Werror -pedantic)
if(WARSHIP_SANITIZERS)
    target_compile_options(d14_g01_pr90_tests PRIVATE
                           -O1 -g -fno-omit-frame-pointer
                           -fsanitize=address,undefined)
    target_link_options(d14_g01_pr90_tests PRIVATE -fsanitize=address,undefined)
endif()
add_test(NAME d14_g01_pr90_every_pixel_exact
         COMMAND d14_g01_pr90_tests "${D14_G01_RAW}")
