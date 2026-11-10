/* D14 TEST ONLY color probe: pure C99 source data, no ESP/RTOS.
 * This component does NOT alter G03 approved assets, RLE or palettes.
 */
#ifndef D14_COLOR_DIAG_H
#define D14_COLOR_DIAG_H
#include <stddef.h>
#include <stdint.h>

#define D14_COLOR_PROFILES 5u
#define D14_COLOR_W 320u
#define D14_COLOR_H 240u
#define D14_COLOR_BAND_ROWS 8u
#define D14_COLOR_BAND_BYTES (D14_COLOR_W*D14_COLOR_BAND_ROWS*2u)

typedef struct {
    char id;          /* A B C D E, observed in this order */
    uint8_t inverted; /* panel inversion, independent of RGB565 payload */
    uint8_t bgr;      /* ILI9341 MADCTL.BGR, independent of payload bytes */
    uint8_t gamma;    /* vendor 0xF2/0x26/0xE0/0xE1 DoomRPG reference */
} D14ColorProfile;

/* A=no changes, B=inversion, C=BGR only, D=inversion+gamma,
 * E=D+BGR. A/B/C are before vendor gamma commands, so no
 * unsupported attempt to "restore default" controller gamma.
 */
int d14_color_profile(unsigned index, D14ColorProfile *out);
extern const uint8_t d14_color_doom_gamma[15];

/* Entire row is filled with direct constant RGB565-BE, bypasses assets.
 * black/white/red/green in row 0, blue/gray/gray ramp/yellow in row 1.
 * 0 success, -1 bad pointer, bounds, capacity or row count.
 */
int d14_color_direct_band(uint16_t y, uint16_t rows,
                          uint8_t *dest, size_t capacity);
uint16_t d14_color_direct_pixel(uint16_t x, uint16_t y);
#endif
