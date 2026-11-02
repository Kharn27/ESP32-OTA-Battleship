idf_component_register(
  SRCS "app_main.c" "t02_probe.c" "d09_probe.c" "t02_cal_probe.c"
  PRIV_REQUIRES warship_core warship_fleet warship_battle warship_captain warship_solo warship_lifecycle warship_palmares warship_palmares_nvs warship_deploy_ui warship_touch t02_touch_map t02_cal warship_lcd log
                esp_driver_spi esp_driver_gpio freertos heap esp_timer
                spi_flash esp_system esp_hw_support
)
