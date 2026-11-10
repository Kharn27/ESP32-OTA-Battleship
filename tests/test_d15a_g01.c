/* D15-A: test the actual pure-C renderer and generated 4-layer Flash data.
 * Detect overwrite, malformed ranges, blending, ordering and side effects.
 * Compiled on GCC/Clang C99 with ASan+UBSan.
 */
#include "d15a_g01.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint8_t band[D15A_BAND_BYTES + 16u];

static uint16_t rgb565_at(const uint8_t *bytes, size_t pixel)
{
    return (uint16_t)(((uint16_t)bytes[pixel * 2u] << 8) | bytes[pixel * 2u + 1u]);
}

static uint16_t expected_pixel(unsigned x, unsigned y)
{
    const D15AAsset *bg = &d15a_assets[D15A_BACKGROUND];
    uint16_t result = rgb565_at(bg->pixels, (size_t)y * D15A_W + x);
    for (unsigned i = D15A_LOGO; i < D15A_IMAGE_COUNT; ++i) {
        const D15AAsset *s = &d15a_assets[i];
        if (x < s->x || y < s->y ||
            x >= (unsigned)s->x + s->w || y >= (unsigned)s->y + s->h)
            continue;
        const size_t idx = ((size_t)(y - s->y) * s->w + x - s->x) * 3u;
        const uint16_t ink = (uint16_t)(((uint16_t)s->pixels[idx] << 8) |
                                         s->pixels[idx + 1u]);
        result = d15a_blend565(result, ink, s->pixels[idx + 2u]);
    }
    return result;
}

static void test_invalid_inputs(void)
{
    memset(band, 0xA5, sizeof band);
    assert(d15a_compose_band(0u, 8u, NULL, D15A_BAND_BYTES) == -1);
    assert(d15a_compose_band(0u, 0u, band, D15A_BAND_BYTES) == -1);
    assert(d15a_compose_band(0u, 9u, band, D15A_BAND_BYTES) == -1);
    assert(d15a_compose_band(240u, 1u, band, D15A_BAND_BYTES) == -1);
    assert(d15a_compose_band(239u, 2u, band, D15A_BAND_BYTES) == -1);
    assert(d15a_compose_band(0u, 8u, band, D15A_BAND_BYTES - 1u) == -1);
    for (size_t i = 0u; i < sizeof band; ++i) assert(band[i] == 0xA5u);
}

static void test_alpha(void)
{
    assert(d15a_blend565(0xF800u, 0x07E0u, 0u) == 0xF800u);
    assert(d15a_blend565(0xF800u, 0x07E0u, 255u) == 0x07E0u);
    assert(d15a_blend565(0xFFFFu, 0x0000u, 127u) != 0xFFFFu);
    assert(d15a_blend565(0xFFFFu, 0x0000u, 127u) != 0x0000u);
    assert(d15a_blend565(0x0000u, 0xFFFFu, 128u)
           == d15a_blend565(0xFFFFu, 0x0000u, 127u));
}

static void test_catalog_geometry(void)
{
    static const unsigned expect_w[4] = {320u, 145u, 140u, 140u};
    static const unsigned expect_h[4] = {240u, 66u, 32u, 30u};
    static const unsigned expect_x[4] = {0u, 88u, 14u, 166u};
    static const unsigned expect_y[4] = {0u, 8u, 198u, 199u};
    size_t total = 0u;
    for (unsigned i = 0; i < 4u; ++i) {
        const D15AAsset *s = &d15a_assets[i];
        assert(s->pixels != NULL && s->w == expect_w[i] && s->h == expect_h[i]);
        assert(s->x == expect_x[i] && s->y == expect_y[i]);
        assert(s->stride == (i == 0u ? 2u : 3u));
        assert(s->bytes == (size_t)s->w * s->h * s->stride);
        total += s->bytes;
        for (unsigned j = 0; j < i; ++j)
            assert(s->pixels != d15a_assets[j].pixels);
    }
    assert(total == 208350u);
}

static void test_stripes(void)
{
    uint64_t fnv = UINT64_C(14695981039346656037);
    unsigned changed_over_wallpaper = 0u, translucent_seen = 0u;
    for (unsigned y = 0u; y < D15A_H; y += D15A_BAND_ROWS) {
        memset(band, 0xA5, sizeof band);
        assert(d15a_compose_band((uint16_t)y, D15A_BAND_ROWS,
                                 band, D15A_BAND_BYTES) == 0);
        for (size_t i = D15A_BAND_BYTES; i < sizeof band; ++i)
            assert(band[i] == 0xA5u);
        for (unsigned row = 0u; row < D15A_BAND_ROWS; ++row) {
            for (unsigned col = 0u; col < D15A_W; ++col) {
                const uint16_t got = rgb565_at(band, (size_t)row * D15A_W + col);
                const uint16_t expected = expected_pixel(col, y + row);
                assert(got == expected);
                const uint16_t bg = rgb565_at(d15a_assets[0].pixels,
                                               ((size_t)y + row) * D15A_W + col);
                if (got != bg) ++changed_over_wallpaper;
                for (unsigned layer = 1u; layer < 4u; ++layer) {
                    const D15AAsset *s = &d15a_assets[layer];
                    if (col >= s->x && col < (unsigned)s->x + s->w &&
                        y + row >= s->y && y + row < (unsigned)s->y + s->h) {
                        const size_t idx = ((size_t)(y + row - s->y) * s->w
                                            + col - s->x) * 3u;
                        if (s->pixels[idx + 2u] > 0u && s->pixels[idx + 2u] < 255u)
                            ++translucent_seen;
                    }
                }
            }
        }
        for (size_t i = 0; i < D15A_BAND_BYTES; ++i) {
            fnv ^= band[i];
            fnv *= UINT64_C(1099511628211);
        }
    }
    assert(changed_over_wallpaper > 500u);
    assert(translucent_seen > 0u);
    memset(band, 0xA5, sizeof band);
    assert(d15a_compose_band(239u, 1u, band, 640u) == 0);
    for (size_t i = 640u; i < sizeof band; ++i) assert(band[i] == 0xA5u);
    printf("D15A_G01_TEST_ONLY four assets, bands=30, modified=%u,"
           " semialpha=%u, frame_fnv64=%016llx PASS\n",
           changed_over_wallpaper, translucent_seen, (unsigned long long)fnv);
}

int main(void)
{
    test_catalog_geometry();
    test_invalid_inputs();
    test_alpha();
    test_stripes();
    return 0;
}
