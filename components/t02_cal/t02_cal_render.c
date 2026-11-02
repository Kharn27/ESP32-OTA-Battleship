#include "t02_cal.h"

#include <stddef.h>

/* 5x7 technical font in Flash; independent of product graphics. */
typedef struct { char ch; uint8_t rows[7]; } Glyph;
static const Glyph font[] = {
 {' ',{0,0,0,0,0,0,0}}, {'0',{14,17,19,21,25,17,14}},
 {'1',{4,12,4,4,4,4,14}}, {'2',{14,17,1,2,4,8,31}},
 {'3',{30,1,1,14,1,1,30}}, {'4',{2,6,10,18,31,2,2}},
 {'5',{31,16,16,30,1,1,30}}, {'/',{1,1,2,4,8,16,16}},
 {'A',{14,17,17,31,17,17,17}}, {'B',{30,17,17,30,17,17,30}},
 {'C',{14,17,16,16,16,17,14}}, {'D',{30,17,17,17,17,17,30}},
 {'E',{31,16,16,30,16,16,31}}, {'F',{31,16,16,30,16,16,16}},
 {'G',{14,17,16,23,17,17,14}}, {'H',{17,17,17,31,17,17,17}},
 {'I',{14,4,4,4,4,4,14}}, {'J',{7,2,2,2,18,18,12}},
 {'K',{17,18,20,24,20,18,17}}, {'L',{16,16,16,16,16,16,31}},
 {'M',{17,27,21,21,17,17,17}}, {'N',{17,25,21,19,17,17,17}},
 {'O',{14,17,17,17,17,17,14}}, {'P',{30,17,17,30,16,16,16}},
 {'Q',{14,17,17,17,21,18,13}}, {'R',{30,17,17,30,20,18,17}},
 {'S',{15,16,16,14,1,1,30}}, {'T',{31,4,4,4,4,4,4}},
 {'U',{17,17,17,17,17,17,14}}, {'V',{17,17,17,17,17,10,4}},
 {'W',{17,17,17,21,21,21,10}}, {'X',{17,17,10,4,10,17,17}},
 {'Y',{17,17,10,4,4,4,4}}, {'Z',{31,1,2,4,8,16,31}}
};
static uint8_t bit(char c, unsigned col, unsigned row)
{
    if (col>=5u || row>=7u) return 0u;
    for (size_t i=0u;i<sizeof(font)/sizeof(font[0]);++i)
        if (font[i].ch==c)
            return (uint8_t)((font[i].rows[row]>>(4u-col))&1u);
    return 0u;
}
static int label(const char *s,unsigned at_x,unsigned at_y,
                 unsigned x,unsigned y)
{
    if (x<at_x || y<at_y || y>=at_y+7u) return 0;
    const unsigned index=(x-at_x)/6u;
    for (unsigned i=0u;s[i]!='\0';++i)
        if (i==index) return bit(s[i],(x-at_x)%6u,y-at_y)!=0u;
    return 0;
}
uint16_t t02_cal_pixel(const T02Cal *cal, uint16_t x, uint16_t y)
{
    if (cal==NULL || x>=320u || y>=240u) return 0u;
    const char *names[5]={"HAUT GAUCHE","HAUT DROIT","CENTRE",
                          "BAS GAUCHE","BAS DROIT"};
    const char *name=cal->point<5u?names[cal->point]:"INVALID";
    char progress[]="POINT 1/5";
    char count[]="PRESS 0/3";
    progress[6]=(char)('1'+cal->point);
    count[6]=(char)('0'+cal->count);
    if (label("T02 CALIBRATION",90u,5u,x,y) ||
        label(progress,90u,21u,x,y) ||
        label(name,90u,35u,x,y) ||
        (cal->phase==T02_CAL_COLLECT && label(count,90u,49u,x,y)) ||
        label("HOLD 2S RETRY",90u,222u,x,y)) return 0xffffu;
    if (cal->phase==T02_CAL_REVIEW &&
        label("MEASURED TAP NEXT",90u,208u,x,y)) return 0x07e0u;
    if (cal->phase==T02_CAL_DONE &&
        (label("DONE SEE UART",90u,188u,x,y) ||
         label("HOLD TO RESTART",90u,208u,x,y))) return 0x07e0u;
    if (cal->phase==T02_CAL_COLLECT &&
        label("TAP TARGET 3 TIMES",90u,208u,x,y)) return 0xffe0u;
    if (y==16u || y==64u || y==200u) return 0x528au;
    if (cal->phase==T02_CAL_DONE) return 0x000cu;
    uint16_t cx,cy;
    t02_cal_target(cal->point,&cx,&cy);
    const int dx=(int)x-(int)cx,dy=(int)y-(int)cy;
    const int dist=dx*dx+dy*dy;
    if ((dist>=144 && dist<=196) || (dx>=-1 && dx<=1 && dy>=-20 && dy<=20) ||
        (dy>=-1 && dy<=1 && dx>=-20 && dx<=20)) return 0xffe0u;
    if (dist<=49) return 0xf800u;
    /* Three verification indicators. */
    for (unsigned i=0u;i<3u;++i) {
        const int indicator_x=273+(int)i*15;
        if ((int)x>=indicator_x && (int)x<indicator_x+9 &&
            y>=31u && y<41u) return i<cal->count?0x07e0u:0x3186u;
    }
    return 0x000cu;
}
