idf_component_register(
  SRCS "app_main.c" "t02_probe.c"
  PRIV_REQUIRES warship_core t02_touch_map warship_lcd log
                esp_driver_spi esp_driver_gpio freertos heap esp_timer
                spi_flash esp_system
)
