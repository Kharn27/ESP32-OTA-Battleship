/* D14 G03: fixed approved art + dynamic PUBLIC human fleet.
 * Flash-only sprites, no heap, no framebuffer, no secret Captain state.
 */
#ifndef D14_G03_H
#define D14_G03_H

#include <stddef.h>
#include <stdint.h>
#include "warship_deploy_ui.h"
#include "t03_decode.h"

#define D14_GRID_X 18u
#define D14_GRID_Y 42u
#define D14_CELL 18u
#define D14_GRID_PX 180u
#define D14_SPRITE_FORMAT_VERSION 1u

/* Each sprite is a rectangle of 3-byte texels:
 * RGB565 BE high/low followed by straight 8-bit alpha.
 * 0=transparent, 255=opaque; intermediate alpha straight blend against
 * the exact CLEAN RGB565 background. Arrays live in Flash.
 * IDs: patrol=0, submarine=1, destroyer=2, cruiser=3, carrier=4.
 * Second index 0 horizontal, 1 vertical. No temporary alpha buffer.
 */
typedef struct {
    const uint8_t *pixels;
    uint8_t w, h;
} D14Sprite;

extern const D14Sprite d14_g03_sprites[5][2];
extern const T03Image d14_g03_image;

WarshipDeployLayout d14_g03_layout(void);
/* Input 320x240 screen coordinate: row A..J down, col 1..10 right.
 * Return 0 on success, -1 outside grid/NULL.
 */
int d14_g03_screen_to_cell(uint16_t x, uint16_t y,
                           uint8_t *col, uint8_t *row);

/* Only DEPLOY frames from public D07 projection are accepted.
 * Validate 5 distinct straight ships of 2,3,3,4,5 = 17 cells.
 * Output is a single borrowed LCD DMA band (max 5120 bytes).
 * On error the band MUST NOT be sent; never re-use after LCD failure.
 * No persistent fleet copy: derive five tiny placements on stack.
 */
int d14_g03_compose(const WarshipDeployFrame *frame,
                    uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                    uint8_t *dest, size_t capacity);

/* AUTO invalidates the complete 180x180 grid, including all old
 * silhouettes (and no other approved PNG pixels).
 */
WarshipDeployRect d14_g03_auto_dirty(void);
#endif
