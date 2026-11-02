#ifndef T02_CAL_H
#define T02_CAL_H

#include <stdint.h>

#define T02_CAL_POINTS 5u
#define T02_CAL_PRESSES 3u
#define T02_CAL_RELEASE_MS 80u
#define T02_CAL_PRESS_MS 40u
#define T02_CAL_RETRY_MS 2200u

typedef struct { uint16_t x, y; } T02CalRaw;
typedef enum { T02_CAL_COLLECT=0, T02_CAL_REVIEW=1, T02_CAL_DONE=2 } T02CalPhase;
typedef enum {
    T02_CAL_NONE=0, T02_CAL_ACCEPT, T02_CAL_REJECT,
    T02_CAL_ADVANCE, T02_CAL_RETRY, T02_CAL_COMPLETE, T02_CAL_RESTART
} T02CalEvent;
typedef struct {
    T02CalRaw samples[T02_CAL_POINTS][T02_CAL_PRESSES];
    uint8_t point, count, phase, contact, invalid, consumed;
    uint64_t down_at, up_at, last_at;
    T02CalRaw latest;
} T02Cal;
typedef struct {
    uint8_t swap;
    uint16_t x0, x1, y0, y1;
    uint16_t max_error_px;
    uint32_t sum_square_error;
} T02CalMapping;
typedef struct {
    T02CalRaw median;
    uint16_t spread_x, spread_y;
} T02CalStats;

void t02_cal_init(T02Cal *cal);
/* Call every poll, including release polls. Valid==0 invalidates the ENTIRE
 * gesture, no matter how brief the SPI fault or PENIRQ rebound. No dynamic
 * allocation; each accepted sample is a distinct press with stable release.
 */
T02CalEvent t02_cal_step(T02Cal *cal, uint8_t down, uint8_t valid,
                         uint16_t raw_x, uint16_t raw_y, uint64_t ms);
int t02_cal_stats(const T02Cal *cal, uint8_t point, T02CalStats *out);
/* Fit both candidate raw-axis assignments to five physical targets. Return
 * 0 only when complete, nondegenerate and one assignment is distinguishable.
 * No mapping is used by D09 until Phase B authorizes it.
 */
int t02_cal_solve(const T02Cal *cal, T02CalMapping *out);
int t02_cal_project(const T02CalMapping *map, uint16_t raw_x,
                    uint16_t raw_y, uint16_t *px, uint16_t *py);
uint16_t t02_cal_pixel(const T02Cal *cal, uint16_t x, uint16_t y);
void t02_cal_target(uint8_t point, uint16_t *x, uint16_t *y);
#endif
