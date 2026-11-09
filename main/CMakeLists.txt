# D14 G03: one ship-free opaque RLE background + five G02 alpha masks.
# Generated in build tree; never changes the PO's canonical design assets.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(D14_PNG "${PROJECT_DIR}/design/ecrans/deploiement/g03_deploiement_320x240.png")
set(D14_SVG "${PROJECT_DIR}/design/ecrans/deploiement/g03_deploiement_320x240.svg")
set(D14_GENERATOR "${PROJECT_DIR}/tools/d14_export.py")
set(D14_GENERATED "${CMAKE_CURRENT_BINARY_DIR}/d14_g03_data.c")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
             "${D14_PNG}" "${D14_SVG}" "${D14_GENERATOR}")
execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${D14_GENERATOR}"
          --png "${D14_PNG}" --svg "${D14_SVG}"
          --output "${D14_GENERATED}"
  RESULT_VARIABLE D14_RESULT OUTPUT_VARIABLE D14_METRICS
  ERROR_VARIABLE D14_ERROR)
if(NOT D14_RESULT EQUAL 0)
  message(FATAL_ERROR "D14 G03 asset extraction failed: ${D14_ERROR}")
endif()
message(STATUS "D14 G03 canonical Flash assets: ${D14_METRICS}")

# T03 opt-in startup benchmark ONLY: no alternate boot, NVS, SPI init.
# Default OFF. Source PNG is never edited; generated C stays in build/.
option(WARSHIP_T03_DIAGNOSTIC "T03 G03 flash formats vs LCD startup diagnostic" OFF)
set(T03_SRCS)
if(WARSHIP_T03_DIAGNOSTIC)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  set(T03_PNG "${PROJECT_DIR}/design/ecrans/deploiement/g03_deploiement_320x240.png")
  set(T03_EXPORT "${PROJECT_DIR}/tools/t03_export.py")
  set(T03_GENERATED "${CMAKE_CURRENT_BINARY_DIR}/t03_g03_data.c")
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${T03_PNG}" "${T03_EXPORT}")
  execute_process(
    COMMAND "${Python3_EXECUTABLE}" "${T03_EXPORT}"
            --input "${T03_PNG}" --output "${T03_GENERATED}"
    RESULT_VARIABLE T03_STATUS OUTPUT_VARIABLE T03_METRICS
    ERROR_VARIABLE T03_ERROR)
  if(NOT T03_STATUS EQUAL 0)
    message(FATAL_ERROR "T03 PNG export failed: ${T03_ERROR}")
  endif()
  message(STATUS "T03 G03 Flash: ${T03_METRICS}")
  list(APPEND T03_SRCS "t03_probe.c" "${T03_GENERATED}")
endif()

idf_component_register(
  SRCS "app_main.c" "d09_probe.c" "d11_runtime.c" "${D14_GENERATED}" ${T03_SRCS}
  PRIV_REQUIRES warship_core warship_fleet warship_battle warship_captain warship_solo warship_lifecycle warship_palmares warship_palmares_nvs warship_event_bridge warship_event_id_nvs warship_deploy_ui warship_combat_ui warship_journey warship_inspection warship_touch t02_touch_map warship_lcd log
                esp_driver_spi esp_driver_gpio freertos heap esp_timer
                spi_flash esp_system esp_hw_support t03_decode d14_g03
)

# An OPT-IN diagnostic build for an explicitly designated test CYD only.
# This mode writes a single synthetic event in namespace warship_d11lab,
# NEVER in the production namespace warship. OFF in all normal CI builds.
option(WARSHIP_D11_NVS_BENCH "Isolated D11 persistent-NVS reception probe" OFF)
if(WARSHIP_D11_NVS_BENCH)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE WARSHIP_D11_NVS_BENCH=1)
endif()

# D13 reception-only deterministic CPU/fleet RNG scenario. Never enabled
# in the production build. Requires D11's isolated test NVS namespace.
option(WARSHIP_D13_CYD_SCENARIO
       "Explicit D13 deterministic CYD test fixture, NO normal gameplay" OFF)
if(WARSHIP_D13_CYD_SCENARIO)
  if(NOT WARSHIP_D11_NVS_BENCH)
    message(FATAL_ERROR
      "D13 scenario must use isolated D11 NVS bench to protect real scores")
  endif()
  target_compile_definitions(${COMPONENT_LIB} PRIVATE
                             WARSHIP_D13_CYD_SCENARIO=1)
endif()

if(WARSHIP_T03_DIAGNOSTIC)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE WARSHIP_T03_DIAGNOSTIC=1)
endif()
