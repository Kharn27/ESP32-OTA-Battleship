# D14 PO backlight reception: experiment-only percent, not a PO choice.
# Change with: idf.py -B build-d14 -D WARSHIP_LCD_BACKLIGHT_PERCENT=40 build
set(WARSHIP_LCD_BACKLIGHT_PERCENT 40 CACHE STRING
    "CYD backlight LEDC trial duty percent (0..100), pending PO acceptance")
if(NOT WARSHIP_LCD_BACKLIGHT_PERCENT MATCHES "^(0|[1-9][0-9]?|100)$")
    message(FATAL_ERROR "WARSHIP_LCD_BACKLIGHT_PERCENT must be 0..100")
endif()
# ESP-IDF dependency scan precedes option evaluation: declare the header
# component explicitly. Its routines remain unused in the default firmware.
idf_component_register(
    SRCS "warship_lcd.c" "warship_lcd_light.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_lcd esp_driver_spi esp_driver_gpio esp_driver_ledc freertos heap d14_color_diag
    PRIV_REQUIRES d14_doom_ref
)
target_compile_definitions(${COMPONENT_LIB} PRIVATE
                           WARSHIP_LCD_BACKLIGHT_PERCENT=${WARSHIP_LCD_BACKLIGHT_PERCENT})
if(WARSHIP_D14_COLOR_DIAGNOSTIC)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
                               WARSHIP_D14_COLOR_DIAGNOSTIC=1)
endif()
if(WARSHIP_D14_DOOM_REFERENCE)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
                               WARSHIP_D14_DOOM_REFERENCE=1)
endif()
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)
