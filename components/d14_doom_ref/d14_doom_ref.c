/* Test-only panel profile transposed as ILI9341 command DATA, not
 * TFT_eSPI/Arduino code. TFT_eSPI V2.5.43 tag commit:
 * 6ae4c97c5a07bd64fce8993e9cfa95d9a81e7c81 (MIT).
 * DoomRPG-RE ESP32/platformio.ini declares bodmer/TFT_eSPI @ ^2.5.43:
 * exact resolved library of the PO's physical firmware is NOT LOCKED.
 */
#include "d14_doom_ref.h"

static const D14DoomCmd init_cmds[] = {
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
static const D14DoomCmd runtime_cmds[] = {
    {0xF2u,1u,0u,{0x00u}},
    {0x26u,1u,0u,{0x01u}},
    {0xE0u,15u,0u,{0x00u,0x15u,0x17u,0x07u,0x11u,0x06u,0x2Bu,0x56u,
                      0x3Cu,0x05u,0x10u,0x0Fu,0x3Fu,0x3Fu,0x0Fu}},
    {0xE1u,15u,0u,{0x00u,0x15u,0x17u,0x07u,0x11u,0x06u,0x2Bu,0x56u,
                      0x3Cu,0x05u,0x10u,0x0Fu,0x3Fu,0x3Fu,0x0Fu}},
    {0x21u,0u,0u,{0}},
    {0x21u,0u,0u,{0}}
};

const D14DoomCmd *d14_doom_init(size_t *count)
{
    if (count == NULL) return NULL;
    *count = sizeof init_cmds / sizeof init_cmds[0];
    return init_cmds;
}

const D14DoomCmd *d14_doom_runtime(size_t *count)
{
    if (count == NULL) return NULL;
    *count = sizeof runtime_cmds / sizeof runtime_cmds[0];
    return runtime_cmds;
}

uint8_t d14_doom_native_landscape_madctl(void)
{
    /* TFT_eSPI V2.5.43 rotation 1: MAD_MV | COLOR_ORDER(BGR). */
    return 0x28u;
}

int d14_doom_native_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                           D14DoomWindow *out)
{
    if (out == NULL || w == 0u || h == 0u ||
        x >= D14_DOOM_W || y >= D14_DOOM_H ||
        w > D14_DOOM_W - (unsigned)x ||
        h > D14_DOOM_H - (unsigned)y)
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

static uint16_t geometry_pixel(uint16_t x, uint16_t y)
{
    /* TL red, TR green, BL blue, BR white. Distinct physical corners. */
    if (x < 6u && y < 6u) return 0xF800u;
    if (x >= D14_DOOM_W - 6u && y < 6u) return 0x07E0u;
    if (x < 6u && y >= D14_DOOM_H - 6u) return 0x001Fu;
    if (x >= D14_DOOM_W - 6u && y >= D14_DOOM_H - 6u) return 0xFFFFu;
    if (x == 0u || x == D14_DOOM_W - 1u ||
        y == 0u || y == D14_DOOM_H - 1u) return 0xFFFFu;
    if (x == 79u || x == 80u || x == 159u || x == 160u ||
        x == 239u || x == 240u || y == 119u || y == 120u)
        return 0x07FFu; /* cyan rectangle boundaries */
    return (((unsigned)x/80u + (unsigned)y/120u) & 1u) ?
           0x4208u : 0x0000u;
}

int d14_doom_geometry_band(uint16_t y, uint16_t rows,
                           uint8_t *dest, size_t capacity)
{
    if (dest == NULL || rows == 0u || rows > D14_DOOM_BAND_ROWS ||
        y >= D14_DOOM_H || (unsigned)y+rows > D14_DOOM_H ||
        capacity < (size_t)D14_DOOM_W*rows*2u)
        return -1;
    for (unsigned row=0u; row<rows; ++row)
        for (unsigned x=0u; x<D14_DOOM_W; ++x) {
            const uint16_t pixel=geometry_pixel((uint16_t)x,
                                                (uint16_t)(y+row));
            const size_t pos=((size_t)row*D14_DOOM_W+x)*2u;
            dest[pos]=(uint8_t)(pixel>>8);
            dest[pos+1u]=(uint8_t)pixel;
        }
    return 0;
}

static uint16_t rgb565(unsigned r, unsigned g, unsigned b)
{
    return (uint16_t)(((r & 0xF8u) << 8) |
                      ((g & 0xFCu) << 3) | (b >> 3));
}

uint16_t d14_doom_midtone_pixel(uint16_t x, uint16_t y)
{
    if (x >= D14_DOOM_W || y >= D14_DOOM_H) return 0u;
    const unsigned tile = ((unsigned)y / 120u) * 4u + (unsigned)x / 80u;
    const unsigned t = (((unsigned)x % 80u) * 255u) / 79u;
    unsigned r = 0u, g = 0u, b = 0u;
    switch (tile) {
    case 0u: r = 96u+t*140u/255u; g = 24u+t*112u/255u; b = 12u+t*48u/255u; break;  /* orange */
    case 1u: r = 32u+t*192u/255u; g = 8u+t*32u/255u; b = 4u+t*16u/255u; break;    /* red */
    case 2u: r = 0u+t*48u/255u; g = 24u+t*112u/255u; b = 48u+t*176u/255u; break; /* blue */
    case 3u: r = g = b = 16u+t*208u/255u; break;                                    /* gray */
    case 4u: r=176u; g=56u; b=38u; break; /* G01 sky sample */
    case 5u: r=24u; g=66u; b=93u; break;  /* G01 sea sample */
    case 6u: r=78u; g=43u; b=18u; break;  /* G01 button sample */
    default: r=221u; g=138u; b=38u; break; /* G01 logo sample */
    }
    return rgb565(r,g,b);
}

int d14_doom_midtone_band(uint16_t y, uint16_t rows,
                          uint8_t *dest, size_t capacity)
{
    if (dest == NULL || rows == 0u || rows > D14_DOOM_BAND_ROWS ||
        y >= D14_DOOM_H || (unsigned)y+rows > D14_DOOM_H ||
        capacity < (size_t)D14_DOOM_W*rows*2u)
        return -1;
    for (unsigned row=0u; row<rows; ++row)
        for (unsigned x=0u; x<D14_DOOM_W; ++x) {
            const uint16_t value=d14_doom_midtone_pixel((uint16_t)x,
                                                        (uint16_t)(y+row));
            const size_t pos=((size_t)row*D14_DOOM_W+x)*2u;
            dest[pos]=(uint8_t)(value>>8);
            dest[pos+1u]=(uint8_t)value;
        }
    return 0;
}
