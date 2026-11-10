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

# The separate D14 probe never replaces or alters the default production boot.
set(D14_COLOR_SRCS)
if(WARSHIP_D14_COLOR_DIAGNOSTIC)
  if(WARSHIP_T03_DIAGNOSTIC OR WARSHIP_D11_NVS_BENCH OR
     WARSHIP_D13_CYD_SCENARIO)
    message(FATAL_ERROR "D14 color probe must be isolated from other diagnostics")
  endif()
  list(APPEND D14_COLOR_SRCS "d14_color_diag.c")
endif()

idf_component_register(
  SRCS "app_main.c" "d09_probe.c" "d11_runtime.c" ${T03_SRCS} ${D14_COLOR_SRCS}
  PRIV_REQUIRES warship_core warship_fleet warship_battle warship_captain warship_solo warship_lifecycle warship_palmares warship_palmares_nvs warship_event_bridge warship_event_id_nvs warship_deploy_ui warship_combat_ui warship_journey warship_inspection warship_touch t02_touch_map warship_lcd log
                esp_driver_spi esp_driver_gpio freertos heap esp_timer
                spi_flash esp_system esp_hw_support t03_decode d14_g03 d14_color_diag
)

if(WARSHIP_D14_COLOR_DIAGNOSTIC)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE
                             WARSHIP_D14_COLOR_DIAGNOSTIC=1)
endif()

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
