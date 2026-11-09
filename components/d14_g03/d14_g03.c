#include "d14_g03.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t col, row, vertical;
} ShipPos;

static int ships_from_public_frame(const WarshipDeployFrame *f,
                                   ShipPos out[5])
{
    static const uint8_t len[5] = {2u,3u,3u,4u,5u};
    if (!f || !out || f->mode != 1u) return -1;
    unsigned total=0;
    for (unsigned i=0;i<5u;++i) {
        unsigned minx=10u, miny=10u, maxx=0u, maxy=0u, seen=0u;
        for (unsigned row=0;row<10u;++row)
            for (unsigned col=0;col<10u;++col)
                if (f->cells[row*10u+col] == i+1u) {
                    if (col<minx) minx=col;
                    if (row<miny) miny=row;
                    if (col>maxx) maxx=col;
                    if (row>maxy) maxy=row;
                    ++seen;
                }
        if (seen != len[i] || minx >= 10u || miny >= 10u)
            return -1;
        const int h=(maxy==miny && maxx-minx+1u==seen);
        const int v=(maxx==minx && maxy-miny+1u==seen);
        if (!h && !v) return -1;
        out[i].col=(uint8_t)minx;
        out[i].row=(uint8_t)miny;
        out[i].vertical=(uint8_t)v;
        total+=seen;
    }
    if (total!=17u) return -1;
    for (unsigned i=0;i<100u;++i)
        if (f->cells[i]>5u) return -1;
    return 0;
}

WarshipDeployLayout d14_g03_layout(void)
{
    WarshipDeployLayout l = {
        .grid={18u,42u,180u,180u},.cell=18u,
        .auto_button={208u,47u,108u,48u},
        .ready_button={208u,104u,108u,54u},
        .return_button={208u,167u,108u,54u},
        /* D15 not authorized: technical HOME START remains explicitly D09. */
        .start_button={231u,99u,82u,40u}
    };
    return l;
}

int d14_g03_screen_to_cell(uint16_t x,uint16_t y,
                           uint8_t *col,uint8_t *row)
{
    if (!col || !row || x<D14_GRID_X || y<D14_GRID_Y ||
        x>=D14_GRID_X+D14_GRID_PX ||
        y>=D14_GRID_Y+D14_GRID_PX) return -1;
    *col=(uint8_t)((x-D14_GRID_X)/D14_CELL);
    *row=(uint8_t)((y-D14_GRID_Y)/D14_CELL);
    return 0;
}

WarshipDeployRect d14_g03_auto_dirty(void)
{
    const WarshipDeployRect r={D14_GRID_X,D14_GRID_Y,
                                D14_GRID_PX,D14_GRID_PX};
    return r;
}

static uint16_t blend_rgb565(uint16_t background, uint16_t ink, unsigned alpha)
{
    if (alpha==255u) return ink;
    if (alpha==0u) return background;
    const unsigned inv=255u-alpha;
    unsigned bgr=(background>>11)&31u, bgg=(background>>5)&63u, bgb=background&31u;
    unsigned fgr=(ink>>11)&31u, fgg=(ink>>5)&63u, fgb=ink&31u;
    /* Channel interpolation in native 5/6/5 precision; no 8-bit
     * per-pixel scratch, fixed 32-bit integer operands. */
    bgr=(bgr*inv+fgr*alpha+127u)/255u;
    bgg=(bgg*inv+fgg*alpha+127u)/255u;
    bgb=(bgb*inv+fgb*alpha+127u)/255u;
    return (uint16_t)((bgr<<11)|(bgg<<5)|bgb);
}

int d14_g03_compose(const WarshipDeployFrame *frame,
                    uint16_t x,uint16_t y,uint16_t w,uint16_t h,
                    uint8_t *dest,size_t capacity)
{
    ShipPos ships[5];
    if (!dest || !w || !h || (size_t)w * h * 2u > 5120u ||
        ships_from_public_frame(frame,ships)!=0)
        return -1;
    /* T03 validates x,y, capacity, offsets and malformed RLE before writing. */
    if (t03_decode_rect(&d14_g03_image,x,y,w,h,dest,capacity)!=0)
        return -1;
    for (unsigned i=0;i<5u;++i) {
        const D14Sprite *sprite=&d14_g03_sprites[i][ships[i].vertical];
        if (!sprite->pixels || !sprite->w || !sprite->h ||
            sprite->w > D14_GRID_PX || sprite->h > D14_GRID_PX)
            return -1;
        const unsigned left=D14_GRID_X+(unsigned)ships[i].col*D14_CELL;
        const unsigned top=D14_GRID_Y+(unsigned)ships[i].row*D14_CELL;
        const unsigned right=left+sprite->w, bottom=top+sprite->h;
        const unsigned from_x=left>x?left:x, from_y=top>y?top:y;
        const unsigned to_x=right<(unsigned)x+w?right:(unsigned)x+w;
        const unsigned to_y=bottom<(unsigned)y+h?bottom:(unsigned)y+h;
        for (unsigned py=from_y;py<to_y;++py)
            for (unsigned px=from_x;px<to_x;++px) {
                const size_t at=((size_t)(py-top)*sprite->w+px-left)*3u;
                const unsigned alpha=sprite->pixels[at+2u];
                if (!alpha) continue;
                const uint16_t ink=(uint16_t)(((uint16_t)sprite->pixels[at]<<8)|
                                              sprite->pixels[at+1u]);
                const size_t to=((size_t)(py-y)*w+px-x)*2u;
                const uint16_t bg=(uint16_t)(((uint16_t)dest[to]<<8)|dest[to+1u]);
                const uint16_t color=blend_rgb565(bg,ink,alpha);
                dest[to]=(uint8_t)(color>>8);
                dest[to+1u]=(uint8_t)color;
            }
    }
    return 0;
}
