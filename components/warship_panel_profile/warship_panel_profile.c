/* PO-validated ILI9341 register data promoted to normal runtime,
 * from D14 DOOM_NATIVE_LANDSCAPE HEAD 9add548f2b255c6565a1c14abdfc7086059ac017.
 * TFT_eSPI V2.5.43 ILI9341_2_DRIVER values under MIT license.
 */
#include "warship_panel_profile.h"

static const WarshipPanelCmd init_cmds[] = {
    {0xCFu,3u,  0u,{0x00u,0xC1u,0x30u}},
    {0xEDu,4u,  0u,{0x64u,0x03u,0x12u,0x81u}},
    {0xE8u,3u,  0u,{0x85u,0x00u,0x78u}},
    {0xCBu,5u,  0u,{0x39u,0x2Cu,0x00u,0x34u,0x02u}},
    {0xF7u,1u,  0u,{0x20u}},
    {0xEAu,2u,  0u,{0x00u,0x00u}},
    {0xC0u,1u,  0u,{0x10u}},
    {0xC1u,1u,  0u,{0x00u}},
    {0xC5u,2u,  0u,{0x30u,0x30u}},
    {0xC7u,1u,  0u,{0xB7u}},
    {0x3Au,1u,  0u,{0x55u}},
    {0x36u,1u,  0u,{0x08u}}, /* TFT 0 portrait, BGR */
    {0xB1u,2u,  0u,{0x00u,0x1Au}},
    {0xB6u,3u,  0u,{0x08u,0x82u,0x27u}},
    {0xF2u,1u,  0u,{0x00u}},
    {0x26u,1u,  0u,{0x01u}},
    {0xE0u,15u,0u,{0x0Fu,0x2Au,0x28u,0x08u,0x0Eu,0x08u,0x54u,0xA9u,
                       0x43u,0x0Au,0x0Fu,0x00u,0x00u,0x00u,0x00u}},
    {0xE1u,15u,0u,{0x00u,0x15u,0x17u,0x07u,0x11u,0x06u,0x2Bu,0x56u,
                       0x3Cu,0x05u,0x10u,0x0Fu,0x3Fu,0x3Fu,0x0Fu}},
    {0x2Bu,4u,  0u,{0x00u,0x00u,0x01u,0x3Fu}},
    {0x2Au,4u,  0u,{0x00u,0x00u,0x00u,0xEFu}},
    {0x11u,0u,120u,{0}}, /* sleep out; TFT_eSPI delay(120) */
    {0x29u,0u,  0u,{0}}, /* display on */
    {0x21u,0u,  0u,{0}}, /* TFT_INVERSION_ON at TFT_eSPI::init */
};

/* DoomRPG platform_video.cpp: F2/26, identical calibrated negative
 * gamma loaded into E0/E1, then invertDisplay(true) sends INVON TWICE.
 * setSwapBytes(true) concerns host uint16_t pushPixels, NOT RGB565BE
 * bytes that this test already places in DMA order.
 */
static const WarshipPanelCmd runtime_cmds[] = {
    {0xF2u,1u,0u,{0x00u}},
    {0x26u,1u,0u,{0x01u}},
    {0xE0u,15u,0u,{0x00u,0x15u,0x17u,0x07u,0x11u,0x06u,0x2Bu,0x56u,
                      0x3Cu,0x05u,0x10u,0x0Fu,0x3Fu,0x3Fu,0x0Fu}},
    {0xE1u,15u,0u,{0x00u,0x15u,0x17u,0x07u,0x11u,0x06u,0x2Bu,0x56u,
                      0x3Cu,0x05u,0x10u,0x0Fu,0x3Fu,0x3Fu,0x0Fu}},
    {0x21u,0u,0u,{0}},
    {0x21u,0u,0u,{0}}
};

const WarshipPanelCmd *warship_panel_init_commands(size_t *count)
{
    if (count == NULL) return NULL;
    *count = sizeof init_cmds / sizeof init_cmds[0];
    return init_cmds;
}

const WarshipPanelCmd *warship_panel_runtime_commands(size_t *count)
{
    if (count == NULL) return NULL;
    *count = sizeof runtime_cmds / sizeof runtime_cmds[0];
    return runtime_cmds;
}

uint8_t warship_panel_landscape_madctl(void)
{
    /* TFT_eSPI V2.5.43 rotation 1: MAD_MV | COLOR_ORDER(BGR). */
    return 0x28u;
}

int warship_panel_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                           WarshipPanelWindow *out)
{
    if (out == NULL || w == 0u || h == 0u ||
        x >= WARSHIP_PANEL_W || y >= WARSHIP_PANEL_H ||
        w > WARSHIP_PANEL_W - (unsigned)x ||
        h > WARSHIP_PANEL_H - (unsigned)y)
        return -1;
    const uint16_t end_x = (uint16_t)(x + w - 1u);
    const uint16_t end_y = (uint16_t)(y + h - 1u);
    out->caset[0]=(uint8_t)(x>>8);
    out->caset[1]=(uint8_t)x;
    out->caset[2]=(uint8_t)(end_x>>8);
    out->caset[3]=(uint8_t)end_x;
    out->raset[0]=(uint8_t)(y>>8);
    out->raset[1]=(uint8_t)y;
    out->raset[2]=(uint8_t)(end_y>>8);
    out->raset[3]=(uint8_t)end_y;
    return 0;
}

