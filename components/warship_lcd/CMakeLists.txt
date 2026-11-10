# Standalone D15-A preview selects the *byte-identical* D14 LCD source,
# without replacing the existing NORMAL driver at all. PR #91 remains blocked.
if(WARSHIP_D15A_G01_TEST_ONLY)
  idf_component_register(
    SRCS "warship_lcd_d15a_preview.c" "warship_lcd_light.c"
    INCLUDE_DIRS "include"
    REQUIRES warship_panel_profile esp_lcd esp_driver_spi
             esp_driver_gpio esp_driver_ledc freertos heap)
else()
  idf_component_register(
      SRCS "warship_lcd.c"
      INCLUDE_DIRS "include"
      REQUIRES esp_lcd esp_driver_spi esp_driver_gpio freertos heap
  )
  target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)
endif()
