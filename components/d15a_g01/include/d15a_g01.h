/* D15-A isolated G01 LCD asset composition (NO product navigation).
 * Generated artwork comes from four individually approved CYD exports only.
 * No allocation, no framebuffer, no microSD; caller owns one 5120-byte band.
 */
#ifndef D15A_G01_H
#define D15A_G01_H
#include <stddef.h>
#include <stdint.h>

#define D15A_W 320u
#define D15A_H 240u
#define D15A_BAND_ROWS 8u
#define D15A_BAND_BYTES (D15A_W * D15A_BAND_ROWS * 2u)
#define D15A_IMAGE_COUNT 4u
#define D15A_BACKGROUND 0u
#define D15A_LOGO 1u
#define D15A_JOUER 2u
#define D15A_PALMARES 3u

typedef struct {
    const uint8_t *pixels;
    size_t bytes;
    uint16_t w, h, x, y;
    uint8_t stride; /* 2=RGB565BE background; 3=RGB565BE + alpha8 */
} D15AAsset;
extern const D15AAsset d15a_assets[D15A_IMAGE_COUNT];

/* Alpha-blend in 5/6/5 native precision, 0 and 255 exact. */
uint16_t d15a_blend565(uint16_t bg, uint16_t ink, uint8_t alpha);

/* Fail-before-write for null/invalid bounds/capacity/asset metadata.
 * 0=success, -1=invalid. Fully overwrites exactly 320*rows*2 bytes.
 */
int d15a_compose_band(uint16_t y, uint16_t rows,
                       uint8_t *dest, size_t capacity);
/* GUI01 single-region composer: w*h*2 <= 5120, fully in 320x240.
 * pressed_layer=D15A_IMAGE_COUNT means NORMAL (bit-identical to D15-A).
 * Otherwise dim only selected button sprite in RGB565 before alpha blend.
 * Caller clips region and owns completed LCD DMA before reuse. */
uint16_t d15a_dim565(uint16_t color, uint8_t brightness);
int d15a_compose_region(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                        unsigned pressed_layer, uint8_t brightness,
                        uint8_t *dest, size_t capacity);
#endif
