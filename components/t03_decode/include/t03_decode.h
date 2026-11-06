/* T03 version 1: exact RGB565 big endian screen/rectangle decoder.
 * No platform headers, no allocation, no globals, no CPU fleet state.
 */
#ifndef T03_DECODE_H
#define T03_DECODE_H
#include <stddef.h>
#include <stdint.h>
#define T03_FORMAT_VERSION 1u
#define T03_G03_WIDTH 320u
#define T03_G03_HEIGHT 240u

typedef enum { T03_RAW565BE = 1, T03_ROW_RLE565BE = 2 } T03Format;
typedef struct {
    const uint8_t *bytes;
    size_t length;
    const uint32_t *row_offsets; /* RLE: height+1 offsets; RAW: NULL */
    size_t row_offset_count;
    uint16_t width;
    uint16_t height;
    T03Format format;
} T03Image;

/* Output: tightly packed RGB565BE rows, (width*height*2) bytes.
 * 0 = success; -1 = invalid argument, bounds, malformed/truncated RLE.
 * The destination MUST NOT be used for display on error.
 * Buffer does not alias source data or row offsets.
 */
int t03_decode_rect(const T03Image *image, uint16_t x, uint16_t y,
                    uint16_t width, uint16_t height,
                    uint8_t *destination, size_t capacity);
#endif
