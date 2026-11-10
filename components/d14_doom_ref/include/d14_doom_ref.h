/* D14 TEST ONLY — register oracle from TFT_eSPI V2.5.43.
 * C99-only data, no Arduino/RTOS dependency, no product LCD changes.
 * SPDX-License-Identifier: MIT — sequence values adapted from Bodmer/TFT_eSPI.
 */
#ifndef D14_DOOM_REF_H
#define D14_DOOM_REF_H
#include <stddef.h>
#include <stdint.h>
#define D14_DOOM_W 320u
#define D14_DOOM_H 240u
#define D14_DOOM_BAND_ROWS 8u
#define D14_DOOM_BAND_BYTES (D14_DOOM_W * D14_DOOM_BAND_ROWS * 2u)

typedef struct {
    uint8_t cmd;
    uint8_t len;
    uint16_t delay_ms; /* delay after the complete command */
    uint8_t data[15];
} D14DoomCmd;

/* Ordered profile: TFT_eSPI ILI9341_2_DRIVER V2.5.43, then
 * DoomRPG platform_video.cpp applyCydInvertedGammaProfile().
 * The LCD transport sends exactly these bytes as command parameters.
 */
const D14DoomCmd *d14_doom_init(size_t *count);
const D14DoomCmd *d14_doom_runtime(size_t *count);
/* TEST ONLY, TFT_eSPI rotation 1: MV=1, BGR=1, MX=MY=0. */
uint8_t d14_doom_native_landscape_madctl(void);

/* Inclusive CASET (0x2A) and RASET (0x2B) windows.
 * Independent host oracle for the esp_lcd draw_bitmap x+w-1/y+h-1 contract.
 */
typedef struct {
    uint8_t caset[4];
    uint8_t raset[4];
} D14DoomWindow;
int d14_doom_native_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                           D14DoomWindow *out);

/* Extra TEST-ONLY first/last pixel & 4x2 boundary screen; no changed PNG. */
int d14_doom_geometry_band(uint16_t y, uint16_t rows,
                           uint8_t *dest, size_t capacity);

/* Deterministic full-width half-tone synthetic diagnostic; wire RGB565BE. */
uint16_t d14_doom_midtone_pixel(uint16_t x, uint16_t y);
int d14_doom_midtone_band(uint16_t y, uint16_t rows,
                          uint8_t *dest, size_t capacity);
#endif
