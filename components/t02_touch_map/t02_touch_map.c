#include "t02_touch_map.h"

#include <stddef.h>

int t02_map_axis(uint16_t raw, uint16_t zero, uint16_t end,
                 uint16_t extent, uint16_t *out)
{
    if (out == NULL || raw > 4095u || zero > 4095u || end > 4095u ||
        zero == end || extent < 2u) {
        return -1;
    }
    const uint32_t range = zero < end ? (uint32_t)(end - zero)
                                      : (uint32_t)(zero - end);
    uint32_t pos;
    if (zero < end) {
        pos = raw <= zero ? 0u : raw >= end ? range : (uint32_t)(raw - zero);
    } else {
        pos = raw >= zero ? 0u : raw <= end ? range : (uint32_t)(zero - raw);
    }
    /* uint32_t is safe even for extent=65535, range=4095. */
    *out = (uint16_t)((pos * (uint32_t)(extent - 1u) + range / 2u) / range);
    return 0;
}

int t02_map_point(const T02TouchMap *map, uint16_t raw_x, uint16_t raw_y,
                  uint16_t *x, uint16_t *y)
{
    if (map == NULL || x == NULL || y == NULL) {
        return -1;
    }
    /* Transactional: an invalid second axis does not mutate either output. */
    uint16_t local_x, local_y;
    if (t02_map_axis(raw_x, map->x_at_left, map->x_at_right,
                     map->width, &local_x) != 0 ||
        t02_map_axis(raw_y, map->y_at_top, map->y_at_bottom,
                     map->height, &local_y) != 0) {
        return -1;
    }
    *x = local_x;
    *y = local_y;
    return 0;
}
