idf_component_register(
  SRCS "app_main.c" "d09_probe.c" "d11_runtime.c"
  PRIV_REQUIRES warship_core warship_fleet warship_battle warship_captain warship_solo warship_lifecycle warship_palmares warship_palmares_nvs warship_event_bridge warship_event_id_nvs warship_deploy_ui warship_combat_ui warship_journey warship_inspection warship_touch t02_touch_map warship_lcd log
                esp_driver_spi esp_driver_gpio freertos heap esp_timer
                spi_flash esp_system esp_hw_support
)

# An OPT-IN diagnostic build for an explicitly designated test CYD only.
# This mode writes a single synthetic event in namespace warship_d11lab,
# NEVER in the production namespace warship. OFF in all normal CI builds.
option(WARSHIP_D11_NVS_BENCH "Isolated D11 persistent-NVS reception probe" OFF)
if(WARSHIP_D11_NVS_BENCH)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE WARSHIP_D11_NVS_BENCH=1)
endif()
