/* GPIO21 backlight LEDC helper; internal to warship_lcd.
 * Entire API is single-task only, never called from LCD DMA callbacks.
 */
#ifndef WARSHIP_LCD_LIGHT_H
#define WARSHIP_LCD_LIGHT_H
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

esp_err_t warship_lcd_light_init(void);
esp_err_t warship_lcd_light_set(bool on);
esp_err_t warship_lcd_light_set_percent(uint8_t percent);
/* Mandatory on every LCD init failure or sticky DMA/panel error.
 * Never assumes gpio_set_level controls a pin assigned to LEDC.
 * Returns error if physical forced-low could not be confirmed by APIs.
 */
esp_err_t warship_lcd_light_force_off(void);
#endif
