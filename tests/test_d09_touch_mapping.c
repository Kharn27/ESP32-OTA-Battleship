/* D09 target touch projection: PO's 15 independent physical taps in #21.
 * These tests exercise the same pure transform as the ESP32 adapter, then
 * the existing B01 dispatcher WITHOUT changing any B01 production code.
 */
#include "t02_touch_map.h"
#include "warship_deploy_ui.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr,"D09_TOUCH_FAIL line %d: %s\n",__LINE__,#x); \
    exit(EXIT_FAILURE); \
} } while (0)
#define EQ(x,y) CHECK((x)==(y))

typedef struct { uint16_t raw_x,raw_y,screen_x,screen_y; } Observation;

static void five_physical_targets(void)
{
    /* These are the medians from the actual 5x3 PO calibration (#21).
     * Error <=2 applies to median targets, NOT per-touch accuracy. */
    static const Observation obs[5] = {
        {661u,489u,24u,24u},
        {659u,3559u,296u,24u},
        {2076u,2001u,160u,120u},
        {3494u,481u,24u,216u},
        {3551u,3524u,296u,216u},
    };
    unsigned squared_error=0u;
    for (unsigned i=0u;i<5u;++i) {
        uint16_t x=999u,y=999u;
        EQ(t02_map_d09_point(obs[i].raw_x,obs[i].raw_y,&x,&y),0);
        const int dx=(int)x-(int)obs[i].screen_x;
        const int dy=(int)y-(int)obs[i].screen_y;
        CHECK(dx>=-2 && dx<=2 && dy>=-2 && dy<=2);
        squared_error+=(unsigned)(dx*dx+dy*dy);
    }
    EQ(squared_error,15u); /* Independently recalculated PO SSE. */

    uint16_t x=777u,y=888u;
    EQ(t02_map_d09_point(2092u,3346u,&x,&y),0);
    EQ(x,279u); EQ(y,120u); /* D09 PRÊT anomaly, now inside hitbox. */
    const WarshipDeployLayout l=warship_deploy_default_layout();
    EQ(warship_deploy_hit(&l,WARSHIP_LIFECYCLE_DEPLOY,x,y),
       WARSHIP_DEPLOY_READY);
    /* Distinguish swap from uncalibrated or direct-axis projection. */
    const T02TouchMap map={215u,3799u,302u,3865u,320u,240u};
    uint16_t direct_x=0u,direct_y=0u;
    EQ(t02_map_point(&map,2092u,3346u,&direct_x,&direct_y),0);
    CHECK(direct_x!=x && direct_y!=y);
    EQ(t02_map_point_swapped(&map,2092u,3346u,&direct_x,&direct_y),0);
    EQ(direct_x,x); EQ(direct_y,y);
}
static void clamps_and_invalid(void)
{
    uint16_t x=777u,y=888u;
    EQ(t02_map_d09_point(302u,215u,&x,&y),0);
    EQ(x,0u); EQ(y,0u);
    EQ(t02_map_d09_point(3865u,3799u,&x,&y),0);
    EQ(x,319u); EQ(y,239u);
    EQ(t02_map_d09_point(1u,1u,&x,&y),0);
    EQ(x,0u); EQ(y,0u);
    EQ(t02_map_d09_point(4094u,4094u,&x,&y),0);
    EQ(x,319u); EQ(y,239u);
    EQ(t02_map_d09_point(2076u,215u,&x,&y),0);
    EQ(x,0u);
    EQ(t02_map_d09_point(302u,2001u,&x,&y),0);
    EQ(y,0u);
    x=777u; y=888u;
    EQ(t02_map_d09_point(4096u,100u,&x,&y),-1);
    EQ(x,777u); EQ(y,888u);
    EQ(t02_map_d09_point(100u,4096u,&x,&y),-1);
    EQ(x,777u); EQ(y,888u);
    EQ(t02_map_d09_point(1u,1u,NULL,&y),-1);
    EQ(x,777u); EQ(y,888u);
    EQ(t02_map_d09_point(1u,1u,&x,NULL),-1);
    EQ(x,777u); EQ(y,888u);
    /* Degenerate endpoints reject atomically, swapped as well as normal. */
    const T02TouchMap bad={215u,215u,302u,3865u,320u,240u};
    EQ(t02_map_point_swapped(&bad,2000u,3000u,&x,&y),-1);
    EQ(x,777u); EQ(y,888u);
    const T02TouchMap off={215u,3799u,302u,3865u,1u,240u};
    EQ(t02_map_point_swapped(&off,2000u,3000u,&x,&y),-1);
    EQ(x,777u); EQ(y,888u);
    const T02TouchMap valid={215u,3799u,302u,3865u,320u,240u};
    EQ(t02_map_point_swapped(NULL,1u,1u,&x,&y),-1);
    EQ(t02_map_point_swapped(&valid,1u,1u,NULL,&y),-1);
    EQ(t02_map_point_swapped(&valid,1u,1u,&x,NULL),-1);
    CHECK(x==777u && y==888u);
}
/* Raw approximation for a requested screen button CENTRE; the actual
 * PO PRET observation is used explicitly below instead of approximating.
 */
static void button_raw(uint16_t px,uint16_t py,uint16_t *rx,uint16_t *ry)
{
    *ry=(uint16_t)(215u+((uint32_t)px*3584u+159u)/319u);
    *rx=(uint16_t)(302u+((uint32_t)py*3563u+119u)/239u);
}
static WarshipDeployAction tick(WarshipDeployInput *in,uint8_t down,
                               uint8_t valid,uint16_t rx,uint16_t ry,
                               uint64_t t,uint8_t phase)
{
    uint16_t x=0u,y=0u;
    const int result=t02_map_d09_point(rx,ry,&x,&y);
    /* Emulate existing D09's saturated ADC rejection. */
    const uint8_t good=(uint8_t)(valid && result==0 &&
              rx>0u && rx<4095u && ry>0u && ry<4095u);
    WarshipDeployAction action=WARSHIP_DEPLOY_RETURN;
    EQ(warship_deploy_input_step(in,down,good,x,y,t,phase,&action),
       WARSHIP_DEPLOY_OK);
    return action;
}
static void gesture(WarshipDeployInput *in,uint16_t rx,uint16_t ry,
                    uint64_t t,uint8_t phase,WarshipDeployAction expected)
{
    EQ(tick(in,1u,1u,rx,ry,t,phase),WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(in,1u,1u,rx,ry,t+40u,phase),expected);
    EQ(tick(in,1u,1u,rx,ry,t+120u,phase),WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(in,0u,1u,rx,ry,t+140u,phase),WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(in,0u,1u,rx,ry,t+200u,phase),WARSHIP_DEPLOY_NO_ACTION);
}
static void actions_with_raw_touch(void)
{
    const WarshipDeployLayout l=warship_deploy_default_layout();
    WarshipDeployInput in;
    EQ(warship_deploy_input_init(&in,&l),WARSHIP_DEPLOY_OK);
    uint16_t rx=0u,ry=0u;
    button_raw(270u,59u,&rx,&ry);
    gesture(&in,rx,ry,100u,WARSHIP_LIFECYCLE_DEPLOY,WARSHIP_DEPLOY_AUTO);
    button_raw(270u,179u,&rx,&ry);
    gesture(&in,rx,ry,400u,WARSHIP_LIFECYCLE_DEPLOY,WARSHIP_DEPLOY_RETURN);
    button_raw(270u,119u,&rx,&ry);
    gesture(&in,rx,ry,700u,WARSHIP_LIFECYCLE_HOME,WARSHIP_DEPLOY_START);
    /* Reproduce the real 1920ms PRET touch: one action, not a repeat. */
    gesture(&in,2092u,3346u,1000u,WARSHIP_LIFECYCLE_DEPLOY,
            WARSHIP_DEPLOY_READY);
    button_raw(270u,59u,&rx,&ry);
    gesture(&in,rx,ry,1300u,WARSHIP_LIFECYCLE_COMBAT,
            WARSHIP_DEPLOY_NO_ACTION);
    button_raw(270u,179u,&rx,&ry);
    gesture(&in,rx,ry,1600u,WARSHIP_LIFECYCLE_COMBAT,
            WARSHIP_DEPLOY_NO_ACTION);
    /* B01: invalid first DOWN + 20ms UP + valid PRÊT must do nothing. */
    EQ(warship_deploy_input_init(&in,&l),WARSHIP_DEPLOY_OK);
    EQ(tick(&in,1u,0u,2092u,3346u,100u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,0u,1u,2092u,3346u,120u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,1u,1u,2092u,3346u,140u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,1u,1u,2092u,3346u,180u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,0u,1u,2092u,3346u,300u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,0u,1u,2092u,3346u,360u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    gesture(&in,2092u,3346u,500u,WARSHIP_LIFECYCLE_DEPLOY,
            WARSHIP_DEPLOY_READY);
}
int main(void)
{
    five_physical_targets();
    clamps_and_invalid();
    actions_with_raw_touch();
    puts("D09 swapped touch mapping and AUTO/RETOUR/START/PRET PASS");
    return EXIT_SUCCESS;
}
