/* Issue #128, portrait TFT technical test only: no production API. */
#ifndef PORTRAITS01_H
#define PORTRAITS01_H
#include <stddef.h>
#include <stdint.h>
#define P01_W 320u
#define P01_H 240u
#define P01_BAND_ROWS 8u
#define P01_BAND_BYTES (P01_W * P01_BAND_ROWS * 2u)
#define P01_COUNT 3u
typedef struct {
    const uint8_t *pixels; /* const Flash RGB565BE + straight alpha8 */
    size_t bytes;
    uint16_t width,height;
    const char *source_sha256; /* original PNG sha256 */
} Portrait01Sprite;
extern const Portrait01Sprite portrait01_sprites[P01_COUNT];
uint16_t portrait01_blend565(uint16_t bg,uint16_t ink,uint8_t alpha);
/* Fully writes rows*320*2 bytes, no heap; invalid args fail before write. */
int portrait01_compose_band(unsigned y,unsigned rows,uint8_t *dst,size_t cap);
#endif
