#include "t02_touch_map.h"

#include <stdint.h>
#include <stdio.h>

static unsigned checks, failures;
#define CHECK(expr) do { ++checks; if (!(expr)) { \
    ++failures; fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #expr); \
} } while (0)

int main(void)
{
    uint16_t out = 999u, x = 777u, y = 888u;
    T02TouchMap normal = {200u, 3800u, 240u, 3760u, 320u, 240u};
    const T02TouchMap reverse = {3800u, 200u, 3760u, 240u, 320u, 240u};

    CHECK(t02_map_axis(200u, 200u, 3800u, 320u, &out) == 0 && out == 0u);
    CHECK(t02_map_axis(3800u, 200u, 3800u, 320u, &out) == 0 && out == 319u);
    CHECK(t02_map_axis(2000u, 200u, 3800u, 320u, &out) == 0 && out == 160u);
    CHECK(t02_map_axis(0u, 200u, 3800u, 320u, &out) == 0 && out == 0u);
    CHECK(t02_map_axis(4095u, 200u, 3800u, 320u, &out) == 0 && out == 319u);
    CHECK(t02_map_axis(3800u, 3800u, 200u, 320u, &out) == 0 && out == 0u);
    CHECK(t02_map_axis(200u, 3800u, 200u, 320u, &out) == 0 && out == 319u);
    CHECK(t02_map_axis(2000u, 3800u, 200u, 320u, &out) == 0 && out == 160u);

    out = 999u;
    CHECK(t02_map_axis(100u, 200u, 200u, 320u, &out) == -1 && out == 999u);
    CHECK(t02_map_axis(100u, 200u, 3800u, 1u, &out) == -1 && out == 999u);
    CHECK(t02_map_axis(100u, 200u, 3800u, 0u, &out) == -1 && out == 999u);
    CHECK(t02_map_axis(4096u, 0u, 4095u, 320u, &out) == -1 && out == 999u);
    CHECK(t02_map_axis(1u, 4096u, 4095u, 320u, &out) == -1 && out == 999u);
    CHECK(t02_map_axis(1u, 0u, 4096u, 320u, &out) == -1 && out == 999u);
    CHECK(t02_map_axis(100u, 200u, 3800u, 320u, NULL) == -1 && out == 999u);
    CHECK(t02_map_axis(4095u, 0u, 4095u, UINT16_MAX, &out) == 0 &&
          out == UINT16_MAX - 1u);
    CHECK(t02_map_point(&normal, 200u, 240u, &x, &y) == 0 && x == 0u && y == 0u);
    CHECK(t02_map_point(&normal, 3800u, 3760u, &x, &y) == 0 &&
          x == 319u && y == 239u);
    CHECK(t02_map_point(&reverse, 3800u, 3760u, &x, &y) == 0 &&
          x == 0u && y == 0u);
    CHECK(t02_map_point(&reverse, 200u, 240u, &x, &y) == 0 &&
          x == 319u && y == 239u);
    x = 777u; y = 888u;
    normal.y_at_bottom = normal.y_at_top;
    CHECK(t02_map_point(&normal, 200u, 1000u, &x, &y) == -1 &&
          x == 777u && y == 888u);
    CHECK(t02_map_point(NULL, 1u, 2u, &x, &y) == -1 &&
          x == 777u && y == 888u);
    CHECK(t02_map_point(&reverse, 1u, 2u, NULL, &y) == -1 &&
          x == 777u && y == 888u);
    CHECK(t02_map_point(&reverse, 1u, 2u, &x, NULL) == -1 &&
          x == 777u && y == 888u);
    printf("T02 touch map C99: %u checks, %u failures\n", checks, failures);
    return failures != 0u;
}
