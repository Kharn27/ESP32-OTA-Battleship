#ifndef T02_TOUCH_MAP_H
#define T02_TOUCH_MAP_H

#include <stdint.h>

/* Pure raw coordinate conversion, independent of ESP-IDF/FreeRTOS.
 * No allocation or global state; physical D09 constants below come from
 * the 15-point PO calibration accepted in Issue #21.
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

/* Direct and swapped axes have identical clamping/validation rules.
 * Output is transactional on failure. */
int t02_map_point_swapped(const T02TouchMap *map,
                          uint16_t raw_x, uint16_t raw_y,
                          uint16_t *x, uint16_t *y);

/* PO-validated CYD profile on ILI9341 console case 10:
 * screen X <- raw Y [215..3799], screen Y <- raw X [302..3865].
 * 320x240, pixels [0..319]x[0..239], clamped outside calibration limits.
 * The raw 0/4095 ADC saturation rejection remains in the D09 caller.
 */
int t02_map_d09_point(uint16_t raw_x, uint16_t raw_y,
                      uint16_t *x, uint16_t *y);

#endif
