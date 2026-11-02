#ifndef WARSHIP_TOUCH_H
#define WARSHIP_TOUCH_H
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    uint16_t x,y;
    uint16_t xs[3],ys[3];
} WarshipTouchSample;
/* Exclusive owner of SPI3 GPIO32/39/25/33 and PENIRQ GPIO36.
 * Initialized once by exactly ONE runtime mode (T02 or D09).
 * T02-B physical calibration NOT established. */
esp_err_t warship_touch_init(void);
bool warship_touch_pressed(void);
/* Sample only when pressed; three X,Y ADC readings and medians.
 * On failure, *out is unchanged and caller must discard whole gesture. */
esp_err_t warship_touch_sample(WarshipTouchSample *out);
#endif
