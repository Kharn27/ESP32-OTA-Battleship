#include "d14_color_diag.h"

const uint8_t d14_color_doom_gamma[15] = {
    0x00u,0x15u,0x17u,0x07u,0x11u,0x06u,0x2bu,0x56u,
    0x3cu,0x05u,0x10u,0x0fu,0x3fu,0x3fu,0x0fu
};

int d14_color_profile(unsigned index, D14ColorProfile *out)
{
    static const D14ColorProfile profiles[D14_COLOR_PROFILES] = {
        {'A',0u,0u,0u}, /* current RGB, no inversion */
        {'B',1u,0u,0u}, /* A + only inversion */
        {'C',0u,1u,0u}, /* A + only BGR */
        {'D',1u,0u,1u}, /* B + exact DoomRPG vendor gamma */
        {'E',1u,1u,1u}  /* D + only BGR */
    };
    if (!out || index >= D14_COLOR_PROFILES) return -1;
    *out=profiles[index];
    return 0;
}

uint16_t d14_color_direct_pixel(uint16_t x,uint16_t y)
{
    /* Diagnostic only: no input outside the real display. */
    static const uint16_t patches[2][4] = {
        {0x0000u,0xffffu,0xf800u,0x07e0u},
        {0x001fu,0x8410u,0x0000u,0xffe0u},
    };
    if(x >= D14_COLOR_W || y >= D14_COLOR_H) return 0u;
    const unsigned row = y/120u, col = x/80u;
    if (row==1u && col==2u) {
        /* A 0..31 black-white R/G/B ramp in the 80px tile.
         * This discriminates gamma/contrast changes with a known input.
         */
        const uint16_t n=(uint16_t)(((unsigned)(x-160u)*31u)/79u);
        const uint16_t g=(uint16_t)((n*63u+15u)/31u);
        return (uint16_t)((n<<11)|(g<<5)|n);
    }
    return patches[row][col];
}

int d14_color_direct_band(uint16_t y,uint16_t rows,
                          uint8_t *dest,size_t capacity)
{
    if(!dest || !rows || rows>D14_COLOR_BAND_ROWS ||
       y>=D14_COLOR_H || (unsigned)y+rows>D14_COLOR_H ||
       capacity < (size_t)D14_COLOR_W*rows*2u)
        return -1;
    for(unsigned r=0u;r<rows;++r)
        for(unsigned x=0u;x<D14_COLOR_W;++x) {
            const uint16_t rgb=d14_color_direct_pixel((uint16_t)x,
                                                      (uint16_t)(y+r));
            const size_t at=((size_t)r*D14_COLOR_W+x)*2u;
            dest[at]=(uint8_t)(rgb>>8);
            dest[at+1u]=(uint8_t)rgb;
        }
    return 0;
}
