/* T03: compare two *exact* representations of the committed G03 PNG,
 * exercising every band and bounded partial rectangles on the host.
 */
#include "t03_decode.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern const uint8_t t03_g03_raw565[153600];
extern const uint8_t t03_g03_rle[];
extern const uint32_t t03_g03_row_offsets[241];
extern const size_t t03_g03_rle_size;

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr,"T03 FAIL line=%d: %s\\n",__LINE__,#x); return 1; \
} } while (0)

static uint8_t actual[5120];
static uint8_t expected[5120];
static T03Image raw = {t03_g03_raw565, 153600u, NULL, 0u,
                       320u, 240u, T03_RAW565BE};
static T03Image rle = {t03_g03_rle, 0u, t03_g03_row_offsets, 241u,
                       320u, 240u, T03_ROW_RLE565BE};

static int compare_rect(unsigned x, unsigned y, unsigned w, unsigned h)
{
    const size_t bytes = (size_t)w * h * 2u;
    CHECK(bytes <= sizeof actual);
    memset(actual, 0xa5, sizeof actual);
    memset(expected, 0x5a, sizeof expected);
    CHECK(t03_decode_rect(&raw, (uint16_t)x, (uint16_t)y, (uint16_t)w,
                          (uint16_t)h, expected, sizeof expected) == 0);
    CHECK(t03_decode_rect(&rle, (uint16_t)x, (uint16_t)y, (uint16_t)w,
                          (uint16_t)h, actual, sizeof actual) == 0);
    CHECK(memcmp(expected, actual, bytes) == 0);
    CHECK(actual[sizeof actual - 1u] == (bytes == sizeof actual ?
          expected[sizeof expected - 1u] : 0xa5u));
    return 0;
}

int main(void)
{
    rle.length = t03_g03_rle_size;
    for (unsigned y = 0; y < 240; y += 8)
        CHECK(compare_rect(0, y, 320, 8) == 0);
    CHECK(compare_rect(0, 0, 1, 1) == 0);
    CHECK(compare_rect(319, 239, 1, 1) == 0);
    CHECK(compare_rect(21, 53, 180, 8) == 0);
    CHECK(compare_rect(22, 24, 19, 19) == 0);
    CHECK(compare_rect(220, 0, 100, 24) == 0);
    CHECK(compare_rect(225, 32, 91, 28) == 0);
    CHECK(compare_rect(0, 232, 320, 8) == 0);

    CHECK(t03_decode_rect(NULL, 0, 0, 1, 1, actual, sizeof actual) != 0);
    CHECK(t03_decode_rect(&raw, 0, 0, 0, 1, actual, sizeof actual) != 0);
    CHECK(t03_decode_rect(&raw, 319, 0, 2, 1, actual, sizeof actual) != 0);
    CHECK(t03_decode_rect(&raw, 0, 239, 1, 2, actual, sizeof actual) != 0);
    CHECK(t03_decode_rect(&raw, 0, 0, 320, 8, actual, 5119) != 0);
    CHECK(t03_decode_rect(&raw, 0, 0, 1, 1, NULL, 2) != 0);
    T03Image bad = raw;
    bad.length--;
    CHECK(t03_decode_rect(&bad, 0, 0, 1, 1, actual, sizeof actual) != 0);
    bad = raw;
    bad.format = (T03Format)99;
    CHECK(t03_decode_rect(&bad, 0, 0, 1, 1, actual, sizeof actual) != 0);

    /* Reject invalid row table, zero run, excessive run, short tuple and
     * incomplete rows without touching the output on an invalid stream. */
    const uint8_t valid[] = {2, 0x12, 0x34, 2, 0xab, 0xcd};
    const uint32_t offsets[] = {0, 6};
    T03Image tiny = {valid, sizeof valid, offsets, 2u,
                     4u, 1u, T03_ROW_RLE565BE};
    memset(actual, 0x5a, sizeof actual);
    CHECK(t03_decode_rect(&tiny, 1, 0, 2, 1, actual, 4) == 0);
    CHECK(actual[0] == 0x12 && actual[1] == 0x34);
    CHECK(actual[2] == 0xab && actual[3] == 0xcd);

    uint8_t broken[6];
    memcpy(broken, valid, sizeof valid);
    tiny.bytes = broken;
    broken[0] = 0;
    memset(actual, 0x5a, sizeof actual);
    CHECK(t03_decode_rect(&tiny, 1, 0, 2, 1, actual, 4) != 0);
    CHECK(actual[0] == 0x5a);
    broken[0] = 5;
    CHECK(t03_decode_rect(&tiny, 1, 0, 2, 1, actual, 4) != 0);
    broken[0] = 2;
    tiny.length = 5;
    CHECK(t03_decode_rect(&tiny, 1, 0, 2, 1, actual, 4) != 0);
    tiny.length = sizeof valid;
    tiny.row_offset_count = 1;
    CHECK(t03_decode_rect(&tiny, 1, 0, 2, 1, actual, 4) != 0);
    tiny.row_offset_count = 2;
    const uint32_t invalid_offsets[] = {0, 9};
    tiny.row_offsets = invalid_offsets;
    CHECK(t03_decode_rect(&tiny, 1, 0, 2, 1, actual, 4) != 0);
    tiny.row_offsets = offsets;
    tiny.width = 5u;
    CHECK(t03_decode_rect(&tiny, 1, 0, 2, 1, actual, 4) != 0);
    puts("T03 PASS: exact full-screen bands, partial ROI, malformed inputs");
    return 0;
}
