#include "t02_cal.h"
#include "t02_touch_map.h"

#include <stddef.h>
#include <string.h>
#include <limits.h>

static uint16_t median3(uint16_t a, uint16_t b, uint16_t c)
{
    if (a>b) { const uint16_t t=a; a=b; b=t; }
    if (b>c) { const uint16_t t=b; b=c; c=t; }
    if (a>b) { const uint16_t t=a; a=b; b=t; }
    return b;
}
void t02_cal_target(uint8_t point, uint16_t *x, uint16_t *y)
{
    static const uint16_t xs[5]={24u,296u,160u,24u,296u};
    static const uint16_t ys[5]={24u,24u,120u,216u,216u};
    if (x!=NULL) *x=point<5u?xs[point]:0u;
    if (y!=NULL) *y=point<5u?ys[point]:0u;
}
void t02_cal_init(T02Cal *cal)
{
    if (cal!=NULL) {
        memset(cal,0,sizeof *cal);
        cal->up_at=UINT64_MAX;
    }
}
T02CalEvent t02_cal_step(T02Cal *cal, uint8_t down, uint8_t valid,
                         uint16_t raw_x, uint16_t raw_y, uint64_t ms)
{
    if (cal==NULL || down>1u || valid>1u) return T02_CAL_REJECT;
    if (ms<cal->last_at) {
        if (cal->contact) cal->invalid=1u;
        return T02_CAL_REJECT;
    }
    cal->last_at=ms;
    if (down) {
        if (!cal->contact) {
            cal->contact=1u;
            cal->down_at=ms;
            cal->up_at=UINT64_MAX;
            cal->invalid=0u;
            cal->consumed=0u;
        } else if (cal->up_at!=UINT64_MAX) {
            /* PENIRQ bounce shorter than RELEASE_MS is still ONE gesture. */
            cal->up_at=UINT64_MAX;
        }
        if (!valid || raw_x==0u || raw_y==0u ||
            raw_x>=4095u || raw_y>=4095u) {
            cal->invalid=1u;
        } else if (!cal->invalid) {
            cal->latest.x=raw_x;
            cal->latest.y=raw_y;
        }
        if (!cal->invalid && !cal->consumed &&
            ms-cal->down_at>=T02_CAL_RETRY_MS) {
            cal->consumed=1u;
            if (cal->phase==T02_CAL_DONE) {
                /* Reset data, but keep the current contact consumed until UP. */
                T02Cal fresh;
                t02_cal_init(&fresh);
                fresh.contact=1u;
                fresh.invalid=0u;
                fresh.consumed=1u;
                fresh.down_at=cal->down_at;
                fresh.last_at=ms;
                *cal=fresh;
                return T02_CAL_RESTART;
            }
            cal->phase=T02_CAL_COLLECT;
            cal->count=0u;
            memset(cal->samples[cal->point],0,sizeof cal->samples[cal->point]);
            return T02_CAL_RETRY;
        }
        return T02_CAL_NONE;
    }
    if (!cal->contact) return T02_CAL_NONE;
    if (cal->up_at==UINT64_MAX) {
        cal->up_at=ms;
        return T02_CAL_NONE;
    }
    if (ms-cal->up_at<T02_CAL_RELEASE_MS) return T02_CAL_NONE;
    cal->contact=0u;
    if (cal->consumed) return T02_CAL_NONE;
    if (cal->invalid || cal->up_at-cal->down_at<T02_CAL_PRESS_MS)
        return T02_CAL_REJECT;
    if (cal->phase==T02_CAL_COLLECT) {
        if (cal->point>=T02_CAL_POINTS || cal->count>=T02_CAL_PRESSES)
            return T02_CAL_REJECT;
        cal->samples[cal->point][cal->count++]=cal->latest;
        if (cal->count==T02_CAL_PRESSES) cal->phase=T02_CAL_REVIEW;
        return T02_CAL_ACCEPT;
    }
    if (cal->phase==T02_CAL_REVIEW) {
        if (cal->point==T02_CAL_POINTS-1u) {
            cal->phase=T02_CAL_DONE;
            return T02_CAL_COMPLETE;
        }
        ++cal->point;
        cal->count=0u;
        cal->phase=T02_CAL_COLLECT;
        return T02_CAL_ADVANCE;
    }
    return T02_CAL_NONE;
}
int t02_cal_stats(const T02Cal *cal, uint8_t point, T02CalStats *out)
{
    if (cal==NULL || out==NULL || point>=T02_CAL_POINTS ||
        (point==cal->point && cal->count<3u && cal->phase!=T02_CAL_DONE) ||
        (point>cal->point && cal->phase!=T02_CAL_DONE)) return -1;
    const T02CalRaw *v=cal->samples[point];
    uint16_t min_x=v[0].x,max_x=v[0].x,min_y=v[0].y,max_y=v[0].y;
    for (unsigned i=1u;i<3u;++i) {
        if (v[i].x<min_x) min_x=v[i].x;
        if (v[i].x>max_x) max_x=v[i].x;
        if (v[i].y<min_y) min_y=v[i].y;
        if (v[i].y>max_y) max_y=v[i].y;
    }
    T02CalStats s={{median3(v[0].x,v[1].x,v[2].x),
                    median3(v[0].y,v[1].y,v[2].y)},
                   (uint16_t)(max_x-min_x),(uint16_t)(max_y-min_y)};
    *out=s;
    return 0;
}
int t02_cal_project(const T02CalMapping *map, uint16_t raw_x,
                    uint16_t raw_y, uint16_t *px, uint16_t *py)
{
    if (map==NULL || px==NULL || py==NULL || map->swap>1u)
        return -1;
    uint16_t lx,ly;
    if (t02_map_axis(map->swap?raw_y:raw_x,map->x0,map->x1,
                     320u,&lx)!=0 ||
        t02_map_axis(map->swap?raw_x:raw_y,map->y0,map->y1,
                     240u,&ly)!=0) return -1;
    *px=lx; *py=ly;
    return 0;
}
static int32_t div_round(int32_t n, int32_t d)
{
    /* d always positive. Round signed numerator to nearest integer. */
    return n>=0?(n+d/2)/d:-((-n+d/2)/d);
}
static uint16_t limit_adc(int32_t v)
{
    return (uint16_t)(v<0?0:v>4095?4095:v);
}
static uint16_t axis(const T02CalStats *s, unsigned pt, unsigned swap,
                     unsigned screen_x)
{
    return screen_x ? (swap?s[pt].median.y:s[pt].median.x)
                    : (swap?s[pt].median.x:s[pt].median.y);
}
static int evaluate(const T02CalStats *s, unsigned swap, T02CalMapping *out)
{
    /* Targets have 24px margins. Estimate raw ADC endpoints at actual pixel
     * 0 and 319/239 by extrapolating all FOUR corner samples.
     */
    const int32_t left=(int32_t)(axis(s,0,swap,1)+axis(s,3,swap,1))/2;
    const int32_t right=(int32_t)(axis(s,1,swap,1)+axis(s,4,swap,1))/2;
    const int32_t top=(int32_t)(axis(s,0,swap,0)+axis(s,1,swap,0))/2;
    const int32_t bottom=(int32_t)(axis(s,3,swap,0)+axis(s,4,swap,0))/2;
    const int32_t dx=right-left, dy=bottom-top;
    if ((dx>-128 && dx<128) || (dy>-128 && dy<128)) return -1;
    T02CalMapping c={0};
    c.swap=(uint8_t)swap;
    c.x0=limit_adc(left-div_round(dx*24,272));
    c.x1=limit_adc(left+div_round(dx*295,272));
    c.y0=limit_adc(top-div_round(dy*24,192));
    c.y1=limit_adc(top+div_round(dy*215,192));
    if (c.x0==c.x1 || c.y0==c.y1) return -1;
    uint32_t sum=0u;
    uint16_t maxerr=0u;
    for (uint8_t i=0u;i<5u;++i) {
        uint16_t tx,ty,px,py;
        t02_cal_target(i,&tx,&ty);
        if (t02_cal_project(&c,s[i].median.x,s[i].median.y,&px,&py)!=0)
            return -1;
        const int32_t ex=(int32_t)px-(int32_t)tx;
        const int32_t ey=(int32_t)py-(int32_t)ty;
        const uint16_t e=(uint16_t)((ex<0?-ex:ex)>(ey<0?-ey:ey)?
                    (ex<0?-ex:ex):(ey<0?-ey:ey));
        if (e>maxerr) maxerr=e;
        sum+=(uint32_t)(ex*ex+ey*ey);
    }
    c.sum_square_error=sum;
    c.max_error_px=maxerr;
    *out=c;
    return 0;
}
int t02_cal_solve(const T02Cal *cal, T02CalMapping *out)
{
    if (cal==NULL || out==NULL || cal->phase!=T02_CAL_DONE) return -1;
    T02CalStats s[5];
    for (uint8_t i=0u;i<5u;++i)
        if (t02_cal_stats(cal,i,&s[i])!=0) return -1;
    T02CalMapping direct,swapped;
    const int d=evaluate(s,0u,&direct), w=evaluate(s,1u,&swapped);
    if (d!=0 && w!=0) return -1;
    if (d!=0) { *out=swapped; return 0; }
    if (w!=0) { *out=direct; return 0; }
    const uint32_t lo=direct.sum_square_error<swapped.sum_square_error?
                      direct.sum_square_error:swapped.sum_square_error;
    const uint32_t hi=direct.sum_square_error>swapped.sum_square_error?
                      direct.sum_square_error:swapped.sum_square_error;
    if (hi-lo<200u) return -1; /* Ambiguous axis assignment, fail closed. */
    *out=direct.sum_square_error<swapped.sum_square_error?direct:swapped;
    return 0;
}
