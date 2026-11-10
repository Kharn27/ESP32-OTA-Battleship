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
/* Normal/A remains the physically referenced case10 (MADCTL.MV=0).
 * TEST-ONLY D14 Doom instead uses native landscape MV=1 / 0x28 BEFORE
 * clearing 320x240. Calling again is a no-op in either variant.
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
#ifdef WARSHIP_D14_COLOR_DIAGNOSTIC
/* A..E strictly in sequence, after prepare_console case10, no DMA pending.
 * TEST ONLY. Any panel cmd error makes LCD sticky and forces LEDC OFF.
 */
esp_err_t warship_lcd_diag_apply_profile(unsigned index);
#endif
#ifdef WARSHIP_D14_DOOM_REFERENCE
/* TEST ONLY, after native-landscape clear and idle DMA.
 * Executes DoomRPG runtime gamma/inversion without overwriting 0x28.
 */
esp_err_t warship_lcd_doom_reference_finish(void);
#endif
bool warship_lcd_info(WarshipLcdInfo *out);
#endif
