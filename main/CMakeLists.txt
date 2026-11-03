idf_component_register(
  SRCS "app_main.c" "d09_probe.c"
  PRIV_REQUIRES warship_core warship_fleet warship_battle warship_captain warship_solo warship_lifecycle warship_palmares warship_palmares_nvs warship_deploy_ui warship_combat_ui warship_touch t02_touch_map warship_lcd log
                esp_driver_spi esp_driver_gpio freertos heap esp_timer
                spi_flash esp_system esp_hw_support
)
