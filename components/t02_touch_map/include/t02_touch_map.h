#ifndef T02_TOUCH_MAP_H
#define T02_TOUCH_MAP_H

#include <stdint.h>

/* Diagnostic conversion only: raw endpoints must be measured on actual CYD.
 * Independent of ESP-IDF and FreeRTOS. No allocation and no global state.
 */
typedef struct {
    uint16_t x_at_left, x_at_right;
    uint16_t y_at_top, y_at_bottom;
    uint16_t width, height;
} T02TouchMap;

/* 0 on success; -1 if out == NULL, endpoints equal, dimensions < 2,
 * endpoints > 4095 or raw > 4095. On error, *out is unchanged.
 * Valid samples outside the measured endpoints are clamped.
 * Either axis may be reversed (descending calibration).
 */
int t02_map_axis(uint16_t raw, uint16_t zero, uint16_t end,
                 uint16_t extent, uint16_t *out);
int t02_map_point(const T02TouchMap *map, uint16_t raw_x, uint16_t raw_y,
                  uint16_t *x, uint16_t *y);

#endif
