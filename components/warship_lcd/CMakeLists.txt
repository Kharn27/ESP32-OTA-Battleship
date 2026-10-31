idf_component_register(
    SRCS "warship_lcd.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_lcd esp_driver_spi esp_driver_gpio freertos heap
)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)
