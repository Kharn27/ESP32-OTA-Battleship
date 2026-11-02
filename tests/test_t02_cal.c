#include "t02_cal.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static unsigned checks, failures;
#define CHECK(z) do { ++checks; if(!(z)) {++failures; fprintf(stderr,"FAIL %u: %s\n",(unsigned)__LINE__,#z);} } while(0)
static uint64_t tick;
static T02CalEvent step(T02Cal *c,int down,int valid,int x,int y,uint64_t at)
{ return t02_cal_step(c,(uint8_t)down,(uint8_t)valid,(uint16_t)x,(uint16_t)y,at); }
static T02CalEvent press(T02Cal *c,int x,int y)
{
    tick+=100u;
    (void)step(c,1,1,x,y,tick);
    CHECK(step(c,1,1,x,y,tick+60u)==T02_CAL_NONE);
    CHECK(step(c,0,1,0,0,tick+80u)==T02_CAL_NONE);
    const T02CalEvent e=step(c,0,1,0,0,tick+160u);
    tick+=160u;
    return e;
}
static void test_gestures(void)
{
    T02Cal c;
    t02_cal_init(&c); tick=0u;
    CHECK(sizeof c<200u);
    CHECK(step(NULL,1,1,10,10,1u)==T02_CAL_REJECT);
    CHECK(step(&c,2,1,10,10,1u)==T02_CAL_REJECT);
    CHECK(step(&c,1,1,2000,1500,100u)==T02_CAL_NONE);
    CHECK(step(&c,0,1,0,0,120u)==T02_CAL_NONE);
    CHECK(step(&c,1,1,2000,1500,140u)==T02_CAL_NONE);/* bounce */
    CHECK(step(&c,0,1,0,0,180u)==T02_CAL_NONE);
    CHECK(step(&c,0,1,0,0,259u)==T02_CAL_NONE);
    CHECK(step(&c,0,1,0,0,260u)==T02_CAL_ACCEPT);
    CHECK(c.count==1u && c.samples[0][0].x==2000u);
    CHECK(step(&c,1,0,0,0,300u)==T02_CAL_NONE);
    CHECK(step(&c,0,1,0,0,330u)==T02_CAL_NONE);
    CHECK(step(&c,0,1,0,0,410u)==T02_CAL_REJECT);
    CHECK(c.count==1u);
    CHECK(step(&c,1,1,1000,1000,500u)==T02_CAL_NONE);
    CHECK(step(&c,1,1,1000,1000,480u)==T02_CAL_REJECT);/* clock regression */
    CHECK(step(&c,0,1,0,0,600u)==T02_CAL_NONE);
    CHECK(step(&c,0,1,0,0,680u)==T02_CAL_REJECT);
    CHECK(c.count==1u);
    tick=800u;
    CHECK(press(&c,2200,1300)==T02_CAL_ACCEPT);
    CHECK(press(&c,2300,1400)==T02_CAL_ACCEPT);
    CHECK(c.phase==T02_CAL_REVIEW && c.count==3u);
    T02CalStats st;
    CHECK(t02_cal_stats(&c,0,&st)==0);
    CHECK(st.median.x==2200u && st.spread_x==300u);
    CHECK(press(&c,0,1)==T02_CAL_REJECT); /* invalid cannot advance */
    CHECK(press(&c,2500,2000)==T02_CAL_ADVANCE);
    CHECK(c.point==1u && c.count==0u);
    /* User can redo a target with an uncalibrated long hold. */
    tick+=100u;
    CHECK(step(&c,1,1,2500,2000,tick)==T02_CAL_NONE);
    CHECK(step(&c,1,1,2500,2000,tick+2200u)==T02_CAL_RETRY);
    CHECK(c.point==1u && c.count==0u);
    CHECK(step(&c,0,1,0,0,tick+2250u)==T02_CAL_NONE);
    CHECK(step(&c,0,1,0,0,tick+2330u)==T02_CAL_NONE);
}
static void set_complete(T02Cal *c,int swapped,int invert_x,int invert_y)
{
    t02_cal_init(c);
    for (uint8_t p=0u;p<5u;++p) {
        uint16_t px,py;
        t02_cal_target(p,&px,&py);
        int rx=300+(int)px*10;
        int ry=350+(int)py*13;
        if(invert_x) rx=4095-rx;
        if(invert_y) ry=4095-ry;
        for(unsigned j=0u;j<3u;++j) {
            c->samples[p][j].x=(uint16_t)(swapped?ry:rx);
            c->samples[p][j].y=(uint16_t)(swapped?rx:ry);
        }
    }
    c->phase=T02_CAL_DONE;
    c->point=4u;
    c->count=3u;
}
static void test_mapping(void)
{
    T02Cal c; T02CalMapping m={0};
    uint16_t x=777u,y=888u;
    const T02CalMapping doom={1u,331u,3802u,372u,3800u,0u,0u};
    CHECK(t02_cal_project(&doom,2092u,3346u,&x,&y)==0);
    CHECK(x>=275u && x<=279u && y>=118u && y<=122u);
    CHECK(t02_cal_project(&doom,0u,0u,&x,&y)==0 && x==0u && y==0u);
    CHECK(t02_cal_project(&doom,4095u,4095u,&x,&y)==0 && x==319u && y==239u);
    for (int swapped=0;swapped<2;++swapped)
        for(int ix=0;ix<2;++ix)
            for(int iy=0;iy<2;++iy) {
                set_complete(&c,swapped,ix,iy);
                CHECK(t02_cal_solve(&c,&m)==0);
                CHECK(m.swap==(uint8_t)swapped);
                CHECK((m.x0>m.x1)==(ix!=0));
                CHECK((m.y0>m.y1)==(iy!=0));
                CHECK(m.max_error_px<=3u);
                CHECK(t02_cal_project(&m,c.samples[2][0].x,
                                          c.samples[2][0].y,&x,&y)==0);
                CHECK(x>=157u && x<=163u && y>=117u && y<=123u);
            }
    CHECK(t02_cal_project(NULL,1u,1u,&x,&y)==-1);
    CHECK(t02_cal_project(&doom,4096u,1u,&x,&y)==-1);
    CHECK(t02_cal_project(&doom,1u,1u,NULL,&y)==-1);
    CHECK(t02_cal_project(&doom,1u,1u,&x,NULL)==-1);
    T02CalMapping bad=doom; bad.x0=bad.x1;
    x=777u; y=888u;
    CHECK(t02_cal_project(&bad,1000u,1000u,&x,&y)==-1 && x==777u && y==888u);
    bad=doom; bad.swap=2u;
    CHECK(t02_cal_project(&bad,1000u,1000u,&x,&y)==-1);
    t02_cal_init(&c);
    CHECK(t02_cal_solve(&c,&m)==-1);
    CHECK(t02_cal_stats(&c,0,NULL)==-1);
    CHECK(t02_cal_stats(&c,5,&(T02CalStats){0})==-1);
    CHECK(t02_cal_pixel(NULL,1,1)==0u);
    CHECK(t02_cal_pixel(&c,320,1)==0u);
    CHECK(t02_cal_pixel(&c,24,24)!=0x000cu);
    CHECK(t02_cal_pixel(&c,160,120)==0x000cu);
    set_complete(&c,0,0,0);
    for(unsigned i=0u;i<3u;++i) {
        c.samples[1][i].x=c.samples[0][i].x;
        c.samples[3][i].y=c.samples[0][i].y;
        c.samples[4][i]=c.samples[0][i];
    }
    CHECK(t02_cal_solve(&c,&m)==-1);
}
static void test_full_15(void)
{
    T02Cal c; t02_cal_init(&c); tick=0u;
    for (uint8_t i=0u;i<5u;++i) {
        for (unsigned j=0u;j<3u;++j)
            CHECK(press(&c,500+(int)i*300+(int)j,
                          700+(int)i*200+(int)j)==T02_CAL_ACCEPT);
        CHECK(c.phase==T02_CAL_REVIEW && c.count==3u);
        CHECK(press(&c,2000,2000)==(i==4u?T02_CAL_COMPLETE:T02_CAL_ADVANCE));
    }
    CHECK(c.phase==T02_CAL_DONE);
    tick+=100u;
    CHECK(step(&c,1,1,2000,2000,tick)==T02_CAL_NONE);
    CHECK(step(&c,1,1,2000,2000,tick+2200u)==T02_CAL_RESTART);
    CHECK(c.point==0u && c.count==0u && c.phase==T02_CAL_COLLECT);
    CHECK(step(&c,0,1,0,0,tick+2240u)==T02_CAL_NONE);
    CHECK(step(&c,0,1,0,0,tick+2320u)==T02_CAL_NONE);
}
int main(void)
{
    test_gestures();test_mapping();test_full_15();
    printf("T02-B host C99 checks=%u failures=%u\n",checks,failures);
    return failures!=0u;
}
