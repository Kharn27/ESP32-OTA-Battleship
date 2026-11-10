/* D15-A: pure C99 four-layer G01 preview over ONE borrowed DMA band.
 * Source pixel data are separate const arrays generated from the four
 * approved assets, never from any composed legacy maquette.
 */
#include "d15a_g01.h"
#include <string.h>

uint16_t d15a_blend565(uint16_t bg, uint16_t ink, uint8_t alpha)
{
    if (alpha == 0u) return bg;
    if (alpha == 255u) return ink;
    const unsigned inv = 255u - alpha;
    const unsigned br = (bg >> 11) & 31u, bg6 = (bg >> 5) & 63u, bb = bg & 31u;
    const unsigned fr = (ink >> 11) & 31u, fg6 = (ink >> 5) & 63u, fb = ink & 31u;
    const unsigned r = (br * inv + fr * alpha + 127u) / 255u;
    const unsigned g = (bg6 * inv + fg6 * alpha + 127u) / 255u;
    const unsigned b = (bb * inv + fb * alpha + 127u) / 255u;
    return (uint16_t)((r << 11) | (g << 5) | b);
}

int d15a_compose_band(uint16_t y, uint16_t rows, uint8_t *dest, size_t capacity)
{
    if (dest == NULL || rows == 0u || rows > D15A_BAND_ROWS ||
        y >= D15A_H || rows > D15A_H - (unsigned)y)
        return -1;
    const size_t n = (size_t)D15A_W * rows * 2u;
    if (n > D15A_BAND_BYTES || capacity < n) return -1;

    /* Validate all four immutable descriptors BEFORE touching caller memory. */
    const D15AAsset *bg = &d15a_assets[D15A_BACKGROUND];
    if (bg->pixels == NULL || bg->w != D15A_W || bg->h != D15A_H ||
        bg->x != 0u || bg->y != 0u || bg->stride != 2u ||
        bg->bytes != (size_t)D15A_W * D15A_H * 2u)
        return -1;
    for (unsigned i = D15A_LOGO; i < D15A_IMAGE_COUNT; ++i) {
        const D15AAsset *s = &d15a_assets[i];
        if (s->pixels == NULL || s->stride != 3u ||
            s->w == 0u || s->h == 0u ||
            s->x >= D15A_W || s->y >= D15A_H ||
            s->w > D15A_W - (unsigned)s->x ||
            s->h > D15A_H - (unsigned)s->y ||
            s->bytes != (size_t)s->w * s->h * 3u)
            return -1;
    }

    memcpy(dest, bg->pixels + (size_t)y * D15A_W * 2u, n);
    for (unsigned i = D15A_LOGO; i < D15A_IMAGE_COUNT; ++i) {
        const D15AAsset *s = &d15a_assets[i];
        const unsigned y_start = y > s->y ? y : s->y;
        const unsigned y_end = (unsigned)y + rows < (unsigned)s->y + s->h
                             ? (unsigned)y + rows : (unsigned)s->y + s->h;
        for (unsigned py = y_start; py < y_end; ++py) {
            for (unsigned px = s->x; px < (unsigned)s->x + s->w; ++px) {
                const size_t src = ((size_t)(py - s->y) * s->w + px - s->x) * 3u;
                const uint8_t alpha = s->pixels[src + 2u];
                if (alpha == 0u) continue;
                const size_t dst = ((size_t)(py - y) * D15A_W + px) * 2u;
                const uint16_t fg = (uint16_t)(((uint16_t)s->pixels[src] << 8) |
                                                s->pixels[src + 1u]);
                if (alpha == 255u) {
                    dest[dst] = (uint8_t)(fg >> 8);
                    dest[dst + 1u] = (uint8_t)fg;
                    continue;
                }
                const uint16_t under = (uint16_t)(((uint16_t)dest[dst] << 8) |
                                                   dest[dst + 1u]);
                const uint16_t result = d15a_blend565(under, fg, alpha);
                dest[dst] = (uint8_t)(result >> 8);
                dest[dst + 1u] = (uint8_t)result;
            }
        }
    }
    return 0;
}
