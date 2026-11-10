# D14 validated visual LCD profile: default LEDC PWM40 (GPIO21, 5 kHz).
# Pilotage may specify another 0..100 percent only for a separate experiment.
set(WARSHIP_LCD_BACKLIGHT_PERCENT 40 CACHE STRING
    "CYD backlight LEDC configured duty percent (0..100, default 40)")
if(NOT WARSHIP_LCD_BACKLIGHT_PERCENT MATCHES "^(0|[1-9][0-9]?|100)$")
    message(FATAL_ERROR "WARSHIP_LCD_BACKLIGHT_PERCENT must be 0..100")
endif()
# ESP-IDF dependency scan precedes option evaluation: declare the header
# component explicitly. Its routines remain unused in the default firmware.
idf_component_register(
    SRCS "warship_lcd.c" "warship_lcd_light.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_lcd esp_driver_spi esp_driver_gpio esp_driver_ledc freertos heap
    PRIV_REQUIRES warship_panel_profile
)
target_compile_definitions(${COMPONENT_LIB} PRIVATE
                           WARSHIP_LCD_BACKLIGHT_PERCENT=${WARSHIP_LCD_BACKLIGHT_PERCENT})
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)
