#include "t03_decode.h"
#include <string.h>

int t03_decode_rect(const T03Image *image, uint16_t x, uint16_t y,
                    uint16_t width, uint16_t height,
                    uint8_t *destination, size_t capacity)
{
    if (!image || !image->bytes || !destination ||
        !image->width || !image->height ||
        !width || !height || x >= image->width || y >= image->height ||
        width > image->width - x || height > image->height - y ||
        (size_t)width > SIZE_MAX / (size_t)height / 2u ||
        capacity < (size_t)width * (size_t)height * 2u)
        return -1;

    /* Validate the complete requested rectangle before writing it.
     * Avoid a partial DMA image following a truncated/malformed RLE row.
     */
    const size_t row_bytes = (size_t)image->width * 2u;
    if (image->format == T03_RAW565BE) {
        if ((size_t)image->height > SIZE_MAX / row_bytes ||
            image->length != (size_t)image->height * row_bytes ||
            image->row_offsets || image->row_offset_count)
            return -1;
        for (uint16_t r = 0; r < height; ++r) {
            const size_t source = ((size_t)(y + r) * image->width + x) * 2u;
            memcpy(destination + (size_t)r * width * 2u,
                   image->bytes + source, (size_t)width * 2u);
        }
        return 0;
    }
    if (image->format != T03_ROW_RLE565BE ||
        !image->row_offsets ||
        image->row_offset_count != (size_t)image->height + 1u ||
        image->row_offsets[0] != 0u ||
        image->row_offsets[image->height] != image->length)
        return -1;
    /* Two passes for atomic rejection; the second fills only the ROI. */
    for (unsigned pass = 0; pass != 2; ++pass) {
        for (uint16_t r = 0; r < height; ++r) {
            const size_t row = (size_t)y + r;
            size_t pos = image->row_offsets[row];
            const size_t end = image->row_offsets[row + 1u];
            size_t column = 0u;
            if (pos > end || end > image->length) return -1;
            while (pos < end) {
                if (end - pos < 3u) return -1;
                const unsigned run = image->bytes[pos];
                if (!run || run > (size_t)image->width - column)
                    return -1;
                const size_t next = column + run;
                if (pass == 1u && next > x && column < (size_t)x + width) {
                    const size_t first = column > x ? column : x;
                    const size_t last = next < (size_t)x + width ?
                                        next : (size_t)x + width;
                    for (size_t c = first; c < last; ++c) {
                        const size_t at = ((size_t)r * width + (c - x)) * 2u;
                        destination[at] = image->bytes[pos + 1u];
                        destination[at + 1u] = image->bytes[pos + 2u];
                    }
                }
                pos += 3u;
                column = next;
            }
            if (column != image->width) return -1;
        }
    }
    return 0;
}
