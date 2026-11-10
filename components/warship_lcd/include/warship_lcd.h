/* T02 CYD LCD transport. Independent WARSHIP implementation using ESP-IDF APIs.
 * Technical behavior checked against the PO-validated Locomotive BASIC console;
 * no source code copied from the GPLv3 repository.
 */
#ifndef WARSHIP_LCD_H
#define WARSHIP_LCD_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#define WARSHIP_LCD_WIDTH 320u
#define WARSHIP_LCD_HEIGHT 240u
#define WARSHIP_LCD_BAND_ROWS 8u
#define WARSHIP_LCD_DMA_BYTES (WARSHIP_LCD_WIDTH * WARSHIP_LCD_BAND_ROWS * 2u)

typedef struct {
    bool initialized;
    bool profile_applied;
    bool dma_fault;
    uint32_t transfers;
    uint32_t timeouts;
    esp_err_t last_error;
} WarshipLcdInfo;

esp_err_t warship_lcd_init(void);
/* Native 320x240 landscape, MADCTL=0x28 before first GRAM window.
 * Doom gamma/inversion after clear; idempotent once prepared.
 */
esp_err_t warship_lcd_prepare_console(uint16_t clear_rgb565);
/* A single borrowed, DMA-capable band. Fill only after previous send returned
 * ESP_OK. Never free or write it after transport fault/timeout.
 */
uint8_t *warship_lcd_band(void);
esp_err_t warship_lcd_send(int x, int y, int width, int height);
/* Backlight GPIO21 active-high assumption pending device verification.
 * Default EXPERIMENTAL PWM is 5 kHz / 40% (configure via
 * -D WARSHIP_LCD_BACKLIGHT_PERCENT=N, 0..100). On/off controls LEDC
 * output enable/stop with idle level 0, not GPIO output when owned by LEDC.
 * Change percent at runtime only from the LCD-owning task; 0 stops PWM.
 * Brightness adjustment does not alter Flash sprites, SPI or MADCTL.
 */
esp_err_t warship_lcd_backlight(bool on);
esp_err_t warship_lcd_backlight_set_percent(uint8_t percent);
bool warship_lcd_info(WarshipLcdInfo *out);
#endif
