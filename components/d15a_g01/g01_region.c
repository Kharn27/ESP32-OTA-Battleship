/* GUI-01: recompose one LCD region from approved Flash assets.
 * Dim selected sprite BEFORE blending over existing background.
 */
#include "d15a_g01.h"
#include <string.h>
uint16_t d15a_dim565(uint16_t color, uint8_t brightness)
{
    const unsigned r=(((color>>11)&31u)*brightness+127u)/255u;
    const unsigned g=(((color>>5)&63u)*brightness+127u)/255u;
    const unsigned b=((color&31u)*brightness+127u)/255u;
    return (uint16_t)((r<<11)|(g<<5)|b);
}
int d15a_compose_region(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                        unsigned pressed_layer, uint8_t brightness,
                        uint8_t *dest, size_t capacity)
{
    if (!dest || !w || !h || x>=D15A_W || y>=D15A_H ||
        w>D15A_W-(unsigned)x || h>D15A_H-(unsigned)y ||
        (size_t)w*h*2u>D15A_BAND_BYTES || capacity<(size_t)w*h*2u ||
        (pressed_layer != D15A_IMAGE_COUNT &&
         pressed_layer != D15A_JOUER && pressed_layer != D15A_PALMARES))
        return -1;
    const D15AAsset *bg=&d15a_assets[D15A_BACKGROUND];
    if (!bg->pixels || bg->x || bg->y || bg->w!=D15A_W || bg->h!=D15A_H ||
        bg->stride!=2u || bg->bytes!=(size_t)D15A_W*D15A_H*2u) return -1;
    for (unsigned i=1;i<D15A_IMAGE_COUNT;++i) {
        const D15AAsset *s=&d15a_assets[i];
        if (!s->pixels || s->stride!=3u || !s->w || !s->h ||
            s->x>=D15A_W || s->y>=D15A_H ||
            s->w>D15A_W-(unsigned)s->x || s->h>D15A_H-(unsigned)s->y ||
            s->bytes!=(size_t)s->w*s->h*3u) return -1;
    }
    for (unsigned row=0;row<h;++row)
        memcpy(dest+(size_t)row*w*2u,
               bg->pixels+((size_t)(y+row)*D15A_W+x)*2u,(size_t)w*2u);
    for (unsigned i=1;i<D15A_IMAGE_COUNT;++i) {
        const D15AAsset *s=&d15a_assets[i];
        const unsigned sx0=x>s->x?x:s->x, sy0=y>s->y?y:s->y;
        const unsigned sx1=x+w<(unsigned)s->x+s->w?x+w:(unsigned)s->x+s->w;
        const unsigned sy1=y+h<(unsigned)s->y+s->h?y+h:(unsigned)s->y+s->h;
        if (sx1<=sx0 || sy1<=sy0) continue;
        for (unsigned py=sy0;py<sy1;++py) for (unsigned px=sx0;px<sx1;++px) {
            const size_t si=((size_t)(py-s->y)*s->w+px-s->x)*3u;
            const unsigned a=s->pixels[si+2u];
            if (!a) continue;
            const size_t di=((size_t)(py-y)*w+px-x)*2u;
            uint16_t fg=(uint16_t)(((uint16_t)s->pixels[si]<<8)|s->pixels[si+1u]);
            if (i==pressed_layer) fg=d15a_dim565(fg,brightness);
            const uint16_t bg565=(uint16_t)(((uint16_t)dest[di]<<8)|dest[di+1u]);
            const uint16_t out=d15a_blend565(bg565,fg,(uint8_t)a);
            dest[di]=(uint8_t)(out>>8);dest[di+1u]=(uint8_t)out;
        }
    }
    return 0;
}
