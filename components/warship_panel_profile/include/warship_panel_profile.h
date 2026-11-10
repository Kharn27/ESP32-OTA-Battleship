/* Production native-C ILI9341 profile for classic 320x240 CYD.
 * Reference ILI9341_2_DRIVER TFT_eSPI V2.5.43 (MIT) with DoomRPG-RE
 * calibrated gamma; no Arduino dependency.
 */
#ifndef WARSHIP_PANEL_PROFILE_H
#define WARSHIP_PANEL_PROFILE_H
#include <stddef.h>
#include <stdint.h>
#define WARSHIP_PANEL_W 320u
#define WARSHIP_PANEL_H 240u
typedef struct {
    uint8_t cmd;
    uint8_t len;
    uint16_t delay_ms;
    uint8_t data[15];
} WarshipPanelCmd;
typedef struct {
    uint8_t caset[4];
    uint8_t raset[4];
} WarshipPanelWindow;
const WarshipPanelCmd *warship_panel_init_commands(size_t *count);
const WarshipPanelCmd *warship_panel_runtime_commands(size_t *count);
uint8_t warship_panel_landscape_madctl(void);
int warship_panel_window(uint16_t x,uint16_t y,uint16_t w,uint16_t h,
                        WarshipPanelWindow *out);
#endif
