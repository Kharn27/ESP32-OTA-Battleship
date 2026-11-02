#include "warship_deploy_ui.h"
#include <stddef.h>
#include <string.h>

/* Tiny bitmap technical font, no graphics assets and no heap. Each
 * row uses five most significant bits, intentionally independent of G03.
 */
typedef struct { char c; uint8_t row[7]; } Glyph;
static const Glyph glyphs[] = {
 {' ',{0,0,0,0,0,0,0}},
 {'0',{14,17,19,21,25,17,14}},
 {'1',{4,12,4,4,4,4,14}},
 {'2',{14,17,1,2,4,8,31}},
 {'3',{30,1,1,14,1,1,30}},
 {'4',{2,6,10,18,31,2,2}},
 {'5',{31,16,16,30,1,1,30}},
 {'6',{14,16,16,30,17,17,14}},
 {'7',{31,1,2,4,8,8,8}},
 {'8',{14,17,17,14,17,17,14}},
 {'9',{14,17,17,15,1,1,14}},
 {'A',{14,17,17,31,17,17,17}},
 {'B',{30,17,17,30,17,17,30}},
 {'C',{14,17,16,16,16,17,14}},
 {'D',{30,17,17,17,17,17,30}},
 {'E',{31,16,16,30,16,16,31}},
 {'F',{31,16,16,30,16,16,16}},
 {'G',{14,17,16,23,17,17,14}},
 {'H',{17,17,17,31,17,17,17}},
 {'I',{14,4,4,4,4,4,14}},
 {'J',{7,2,2,2,18,18,12}},
 {'K',{17,18,20,24,20,18,17}},
 {'L',{16,16,16,16,16,16,31}},
 {'M',{17,27,21,21,17,17,17}},
 {'N',{17,25,21,19,17,17,17}},
 {'O',{14,17,17,17,17,17,14}},
 {'P',{30,17,17,30,16,16,16}},
 {'Q',{14,17,17,17,21,18,13}},
 {'R',{30,17,17,30,20,18,17}},
 {'S',{15,16,16,14,1,1,30}},
 {'T',{31,4,4,4,4,4,4}},
 {'U',{17,17,17,17,17,17,14}},
 {'V',{17,17,17,17,17,10,4}},
 {'W',{17,17,17,21,21,21,10}},
 {'X',{17,17,10,4,10,17,17}},
 {'Y',{17,17,10,4,4,4,4}},
 {'Z',{31,1,2,4,8,16,31}},
 {'-',{0,0,0,31,0,0,0}},
};
static const uint16_t ship_colors[6] = {
    0x1143u, 0x07ffu, 0xfbe0u, 0xf81fu, 0x07e0u, 0xfd20u
};
static int glyph_bit(char c, unsigned col, unsigned row)
{
    if (col >= 5u || row >= 7u) return 0;
    for (size_t i=0u; i<sizeof glyphs / sizeof glyphs[0]; ++i)
        if (glyphs[i].c == c)
            return ((glyphs[i].row[row] >> (4u-col)) & 1u) != 0u;
    return 0;
}
static int text_bit(const char *s, unsigned at_x, unsigned at_y,
                    unsigned x, unsigned y)
{
    if (x < at_x || y < at_y || y >= at_y+7u) return 0;
    const unsigned glyph_index = (x-at_x)/6u;
    const size_t len = strlen(s);
    return glyph_index < len &&
           glyph_bit(s[glyph_index],(x-at_x)%6u,y-at_y);
}
static int in_rect(WarshipDeployRect r,unsigned x,unsigned y)
{
    return x>=r.x && y>=r.y && x<(unsigned)r.x+r.w &&
           y<(unsigned)r.y+r.h;
}
static int border(WarshipDeployRect r,unsigned x,unsigned y)
{
    return in_rect(r,x,y) &&
           (x==r.x || y==r.y ||
            x+1u==(unsigned)r.x+r.w ||
            y+1u==(unsigned)r.y+r.h);
}
static uint16_t button(WarshipDeployRect r,const char *label,
                       unsigned x,unsigned y,uint16_t fill)
{
    if (!in_rect(r,x,y)) return 0x0000u;
    if (border(r,x,y)) return 0xffffu;
    const unsigned glyph_len=(unsigned)strlen(label)*6u-1u;
    const unsigned tx=r.x+(r.w-glyph_len)/2u;
    const unsigned ty=r.y+(r.h-7u)/2u;
    if (text_bit(label,tx,ty,x,y)) return 0xffffu;
    return fill;
}

uint16_t warship_deploy_pixel(const WarshipDeployFrame *f,
                              const WarshipDeployLayout *l,
                              uint16_t x, uint16_t y)
{
    if (f==NULL || !warship_deploy_layout_valid(l) ||
        x>=WARSHIP_DEPLOY_W || y>=WARSHIP_DEPLOY_H)
        return 0x0000u;

    if (f->mode == 0u) {
        if (text_bit("D09 TECH",35u,35u,x,y) ||
            text_bit("DEPLOIEMENT",35u,58u,x,y) ||
            text_bit("DEMO",35u,82u,x,y))
            return 0xffffu;
        if (in_rect(l->start_button,x,y))
            return button(l->start_button,"START",x,y,0x0280u);
        if (text_bit("NON CALIBRE",35u,187u,x,y))
            return 0xffe0u;
        return 0x000cu;
    }
    if (text_bit("DEPLOIEMENT",23u,3u,x,y) ||
        text_bit("D09",260u,9u,x,y))
        return 0xffffu;

    for (unsigned i=0u;i<10u;++i) {
        const unsigned cx=l->grid.x+i*l->cell+(l->cell-5u)/2u;
        const unsigned cy=l->grid.y+i*l->cell+(l->cell-7u)/2u;
        if (glyph_bit((char)('A'+i),
                      x>=cx?x-cx:100u,y>=12u?y-12u:100u))
            return 0xffffu;
        if (i==9u) {
            if (text_bit("10",2u,cy,x,y)) return 0xffffu;
        } else {
            if (glyph_bit((char)('1'+i),x>=8u?x-8u:100u,
                          y>=cy?y-cy:100u)) return 0xffffu;
        }
    }
    if (in_rect(l->grid,x,y)) {
        const unsigned dx=x-l->grid.x, dy=y-l->grid.y;
        const unsigned ix=dx/l->cell,iy=dy/l->cell;
        if (dx%l->cell==0u || dy%l->cell==0u ||
            dx%l->cell+1u==l->cell ||
            dy%l->cell+1u==l->cell) return 0x9fffu;
        const uint8_t cell=f->cells[iy*10u+ix];
        return ship_colors[cell<6u?cell:0u];
    }

    if (f->mode==2u) {
        if (text_bit("DEMO",247u,50u,x,y) ||
            text_bit("COMBAT",236u,73u,x,y) ||
            text_bit("NON",254u,95u,x,y) ||
            text_bit("INTEGRE",235u,118u,x,y) ||
            text_bit("RESET",245u,175u,x,y))
            return 0xffffu;
        return 0x000cu;
    }
    if (in_rect(l->auto_button,x,y))
        return button(l->auto_button,"AUTO",x,y,0x0220u);
    if (in_rect(l->ready_button,x,y))
        return button(l->ready_button,"PRET",x,y,0x0260u);
    if (in_rect(l->return_button,x,y))
        return button(l->return_button,"RETOUR",x,y,0x3000u);
    if (text_bit("17 CASES",233u,215u,x,y))
        return 0xffffu;
    return 0x000cu;
}
