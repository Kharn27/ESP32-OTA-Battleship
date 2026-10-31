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
/* Configure ONLY the physically referenced console boot profile 10.
 * In this CYD-specific profile, a 320x240 window is addressed with
 * MADCTL.MV=0. Do not substitute a generic landscape rotation.
 * Calling the function again is a no-op (do not reapply MADCTL).
 */
esp_err_t warship_lcd_prepare_console(uint16_t clear_rgb565);
/* A single borrowed, DMA-capable band. Fill only after previous send returned
 * ESP_OK. Never free or write it after transport fault/timeout.
 */
uint8_t *warship_lcd_band(void);
esp_err_t warship_lcd_send(int x, int y, int width, int height);
esp_err_t warship_lcd_backlight(bool on);
bool warship_lcd_info(WarshipLcdInfo *out);
#endif
