#include "warship_deploy_ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr,"FAIL %s:%d %s\n",__FILE__,__LINE__,#x); \
    exit(EXIT_FAILURE); \
} } while (0)
#define EQ(a,b) CHECK((a)==(b))
#define OK(x) EQ((x),WARSHIP_DEPLOY_OK)
typedef struct { uint32_t n,offset,fail_at; } FakeRandom;
static int random_u32(void *ctx,uint32_t *out)
{
    FakeRandom *r=(FakeRandom *)ctx;
    ++r->n;
    if(r->fail_at!=0u && r->n==r->fail_at)return -1;
    *out=r->offset+20u*(r->n-1u);
    return 0;
}
static void same(const void *a,const void *b,size_t n)
{
    CHECK(memcmp(a,b,n)==0);
}
static void observe(const WarshipLifecycle *g,WarshipLifecyclePublic *v)
{
    CHECK(warship_lifecycle_observe(g,v)==WARSHIP_LIFECYCLE_OK);
}
static WarshipDeployAction tick(WarshipDeployInput *in,uint8_t down,
    uint8_t valid,uint16_t x,uint16_t y,uint64_t ms,uint8_t phase)
{
    WarshipDeployAction action=WARSHIP_DEPLOY_RETURN;
    OK(warship_deploy_input_step(in,down,valid,x,y,ms,phase,&action));
    return action;
}
static void geometry(void)
{
    WarshipDeployLayout l=warship_deploy_default_layout();
    CHECK(warship_deploy_layout_valid(&l));
    EQ(l.grid.w,190u); EQ(l.grid.h,190u);
    EQ((unsigned)l.cell*10u,190u);
    EQ(warship_deploy_hit(&l,WARSHIP_LIFECYCLE_HOME,270u,119u),WARSHIP_DEPLOY_START);
    EQ(warship_deploy_hit(&l,WARSHIP_LIFECYCLE_DEPLOY,270u,59u),WARSHIP_DEPLOY_AUTO);
    EQ(warship_deploy_hit(&l,WARSHIP_LIFECYCLE_DEPLOY,270u,119u),WARSHIP_DEPLOY_READY);
    EQ(warship_deploy_hit(&l,WARSHIP_LIFECYCLE_DEPLOY,270u,179u),WARSHIP_DEPLOY_RETURN);
    EQ(warship_deploy_hit(&l,WARSHIP_LIFECYCLE_DEPLOY,231u,60u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(warship_deploy_hit(&l,WARSHIP_LIFECYCLE_DEPLOY,232u,60u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(warship_deploy_hit(&l,WARSHIP_LIFECYCLE_DEPLOY,311u,60u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(warship_deploy_hit(&l,WARSHIP_LIFECYCLE_DEPLOY,270u,39u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(warship_deploy_hit(&l,WARSHIP_LIFECYCLE_DEPLOY,270u,78u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(warship_deploy_hit(&l,WARSHIP_LIFECYCLE_DEPLOY,250u,85u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(warship_deploy_hit(&l,WARSHIP_LIFECYCLE_DEPLOY,220u,100u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(warship_deploy_hit(&l,WARSHIP_LIFECYCLE_DEPLOY,319u,239u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(warship_deploy_hit(&l,WARSHIP_LIFECYCLE_COMBAT,270u,59u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(warship_deploy_hit(&l,WARSHIP_LIFECYCLE_HOME,270u,59u),WARSHIP_DEPLOY_NO_ACTION);
    CHECK(!warship_deploy_layout_valid(NULL));
    l.cell=20u; CHECK(!warship_deploy_layout_valid(&l));
    l=warship_deploy_default_layout();
    l.auto_button.x=190u; CHECK(!warship_deploy_layout_valid(&l));
    l=warship_deploy_default_layout();
    l.ready_button.x=310u; CHECK(!warship_deploy_layout_valid(&l));
    l=warship_deploy_default_layout();
    l.return_button.y=130u; CHECK(!warship_deploy_layout_valid(&l));
}
static void input(void)
{
    WarshipDeployLayout l=warship_deploy_default_layout();
    WarshipDeployInput in;
    memset(&in,0x9c,sizeof in);
    OK(warship_deploy_input_init(&in,&l));
    EQ(tick(&in,1u,1u,270u,59u,100u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,1u,1u,270u,59u,119u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,1u,1u,300u,179u,140u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_AUTO); /* first point wins, moving doesn't change */
    for (unsigned k=0u;k<30u;++k)
        EQ(tick(&in,1u,1u,270u,119u,160u+k*20u,WARSHIP_LIFECYCLE_DEPLOY),
           WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,0u,1u,0u,0u,800u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    /* Brief false release and re-press cannot rearm. */
    EQ(tick(&in,1u,1u,270u,59u,820u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,1u,1u,270u,59u,900u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,0u,1u,0u,0u,940u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,0u,1u,0u,0u,1000u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,1u,1u,270u,119u,1040u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,1u,1u,270u,119u,1081u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_READY);
    EQ(tick(&in,1u,1u,270u,59u,1200u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,0u,1u,0u,0u,1220u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,0u,1u,0u,0u,1280u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    /* Bad SPI / invalid map poisons the WHOLE gesture. */
    EQ(tick(&in,1u,0u,270u,59u,1400u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,1u,1u,270u,59u,1500u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,0u,1u,0u,0u,1550u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,0u,1u,0u,0u,1610u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,1u,1u,10u,60u,1700u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,1u,1u,270u,59u,1750u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,0u,1u,0u,0u,1800u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,0u,1u,0u,0u,1860u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,1u,1u,270u,59u,1900u,WARSHIP_LIFECYCLE_COMBAT),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,1u,1u,270u,59u,1945u,WARSHIP_LIFECYCLE_COMBAT),
       WARSHIP_DEPLOY_NO_ACTION);
    WarshipDeployInput saved=in;
    WarshipDeployAction out=WARSHIP_DEPLOY_RETURN;
    EQ(warship_deploy_input_step(&in,1u,1u,270u,59u,1800u,
       WARSHIP_LIFECYCLE_DEPLOY,&out),WARSHIP_DEPLOY_BAD_ARGUMENT);
    same(&in,&saved,sizeof in); EQ(out,WARSHIP_DEPLOY_RETURN);
    EQ(warship_deploy_input_step(&in,2u,1u,270u,59u,2000u,
       WARSHIP_LIFECYCLE_DEPLOY,&out),WARSHIP_DEPLOY_BAD_ARGUMENT);
    same(&in,&saved,sizeof in); EQ(out,WARSHIP_DEPLOY_RETURN);
}
/* Drive the same dispatcher->D07 boundary as the ESP32 adapter. A
 * spurious READY would mutate game.phase and fail before the memcmp. */
static WarshipDeployAction touch_game(
    WarshipDeployInput *in, WarshipLifecycle *game,
    uint8_t pressed, uint8_t valid, uint16_t x, uint16_t y,
    uint64_t ms)
{
    const WarshipDeployAction action = tick(in, pressed, valid, x, y, ms,
                                           (uint8_t)game->phase);
    if (action != WARSHIP_DEPLOY_NO_ACTION)
        OK(warship_deploy_apply(game, action, NULL, NULL, NULL, NULL));
    return action;
}
static void ready_game(WarshipLifecycle *game)
{
    FakeRandom human = {0u, 0u, 0u};
    FakeRandom captain = {0u, 10u, 0u};
    EQ(warship_lifecycle_init(game), WARSHIP_LIFECYCLE_OK);
    OK(warship_deploy_apply(game, WARSHIP_DEPLOY_START,
                           random_u32, &human, random_u32, &captain));
    EQ(game->phase, WARSHIP_LIFECYCLE_DEPLOY);
}
static void invalid_release_bounce(void)
{
    WarshipLifecycle game;
    ready_game(&game);
    const WarshipLifecycle before = game;
    WarshipDeployInput in;
    const WarshipDeployLayout l = warship_deploy_default_layout();
    OK(warship_deploy_input_init(&in, &l));

    /* Exact Pilotage B01 reproduction: invalid at 100, UP only 20 ms,
     * valid new DOWN at 140, then hold 40+ ms over PRET. */
    EQ(touch_game(&in,&game,1u,0u,270u,119u,100u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(in.first_valid,0u);
    EQ(in.armed,0u); /* Invalid first sample reserves the gesture. */
    EQ(touch_game(&in,&game,0u,1u,0u,0u,120u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,140u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,180u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,250u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(in.first_valid,0u); EQ(in.armed,0u);
    same(&game,&before,sizeof game);

    /* Even 59 ms UP followed by another press is NOT a release. */
    EQ(touch_game(&in,&game,0u,1u,0u,0u,300u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,0u,1u,0u,0u,359u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(in.armed,0u);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,360u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,400u),WARSHIP_DEPLOY_NO_ACTION);
    same(&game,&before,sizeof game);

    /* Continuous UP for exactly 60 ms finally arms a NEW gesture. */
    EQ(touch_game(&in,&game,0u,1u,0u,0u,420u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,0u,1u,0u,0u,479u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(in.armed,0u);
    EQ(touch_game(&in,&game,0u,1u,0u,0u,480u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(in.armed,1u);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,500u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,539u),WARSHIP_DEPLOY_NO_ACTION);
    same(&game,&before,sizeof game);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,540u),WARSHIP_DEPLOY_READY);
    EQ(game.phase,WARSHIP_LIFECYCLE_COMBAT);
    const WarshipLifecycle after_ready = game;
    EQ(touch_game(&in,&game,1u,1u,270u,119u,800u),WARSHIP_DEPLOY_NO_ACTION);
    same(&game,&after_ready,sizeof game); /* PRET emitted once, not twice. */
}

static void invalid_midpress_bounce(void)
{
    WarshipLifecycle game;
    ready_game(&game);
    const WarshipLifecycle before = game;
    WarshipDeployInput in;
    const WarshipDeployLayout l = warship_deploy_default_layout();
    OK(warship_deploy_input_init(&in, &l));

    /* First point valid; a later SPI or mapping failure BEFORE 40 ms
     * poisons that gesture. A bounced release cannot clear the latch. */
    EQ(touch_game(&in,&game,1u,1u,270u,119u,100u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,0u,270u,119u,120u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(in.first_valid,0u);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,139u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,0u,1u,0u,0u,140u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,150u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,190u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,250u),WARSHIP_DEPLOY_NO_ACTION);
    same(&game,&before,sizeof game);
    EQ(touch_game(&in,&game,0u,1u,0u,0u,260u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,0u,1u,0u,0u,320u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,330u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,370u),WARSHIP_DEPLOY_READY);
    EQ(game.phase,WARSHIP_LIFECYCLE_COMBAT);
}

static void valid_short_release_bounce(void)
{
    WarshipLifecycle game;
    ready_game(&game);
    const WarshipLifecycle before=game;
    WarshipDeployInput in;
    const WarshipDeployLayout l=warship_deploy_default_layout();
    OK(warship_deploy_input_init(&in,&l));
    EQ(touch_game(&in,&game,1u,1u,270u,119u,100u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,0u,1u,0u,0u,120u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,130u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,170u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,300u),WARSHIP_DEPLOY_NO_ACTION);
    same(&game,&before,sizeof game);
    EQ(touch_game(&in,&game,0u,1u,0u,0u,310u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,0u,1u,0u,0u,370u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,390u),WARSHIP_DEPLOY_NO_ACTION);
    EQ(touch_game(&in,&game,1u,1u,270u,119u,430u),WARSHIP_DEPLOY_READY);
    EQ(game.phase,WARSHIP_LIFECYCLE_COMBAT);

    /* Regressive timestamp while PENIRQ is UP must not accidentally
     * underflow the release time and rearm the dispatcher. */
    OK(warship_deploy_input_init(&in,&l));
    EQ(tick(&in,1u,1u,270u,119u,500u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    EQ(tick(&in,0u,1u,0u,0u,520u,WARSHIP_LIFECYCLE_DEPLOY),
       WARSHIP_DEPLOY_NO_ACTION);
    const WarshipDeployInput saved=in;
    WarshipDeployAction out=WARSHIP_DEPLOY_RETURN;
    EQ(warship_deploy_input_step(&in,0u,1u,0u,0u,519u,
          WARSHIP_LIFECYCLE_DEPLOY,&out),WARSHIP_DEPLOY_BAD_ARGUMENT);
    same(&in,&saved,sizeof in);
    EQ(out,WARSHIP_DEPLOY_RETURN);
}
static void lifecycle_and_frame(void)
{
    WarshipLifecycle game;
    CHECK(warship_lifecycle_init(&game)==WARSHIP_LIFECYCLE_OK);
    WarshipLifecyclePublic pub={0};
    observe(&game,&pub);
    WarshipDeployFrame frame;
    memset(&frame,0xa3,sizeof frame);
    OK(warship_deploy_make_frame(&pub,&frame));
    EQ(frame.mode,0u);
    for(unsigned i=0u;i<100u;++i) EQ(frame.cells[i],0u);
    FakeRandom human={0u,0u,0u},captain={0u,10u,0u};
    CHECK(warship_deploy_apply(&game,WARSHIP_DEPLOY_AUTO,
                               random_u32,&human,random_u32,&captain)!=
          WARSHIP_DEPLOY_OK);
    OK(warship_deploy_apply(&game,WARSHIP_DEPLOY_START,
                           random_u32,&human,random_u32,&captain));
    EQ(game.phase,WARSHIP_LIFECYCLE_DEPLOY);
    EQ(human.n,5u); EQ(captain.n,5u);
    observe(&game,&pub);
    EQ(pub.has_human_fleet,1u);
    EQ(pub.has_battle,0u);
    EQ(pub.can_reveal_captain,0u);
    OK(warship_deploy_make_frame(&pub,&frame));
    EQ(frame.mode,1u);
    unsigned count=0u;
    unsigned ships[5]={0};
    for(unsigned i=0u;i<100u;++i) {
        if(frame.cells[i]) {
            CHECK(frame.cells[i]<=5u);
            ++ships[frame.cells[i]-1u]; ++count;
        }
    }
    EQ(count,17u);
    EQ(ships[0],2u); EQ(ships[1],3u); EQ(ships[2],3u);
    EQ(ships[3],4u); EQ(ships[4],5u);
    WarshipDeployLayout layout=warship_deploy_default_layout();
    const uint16_t water=warship_deploy_pixel(&frame,&layout,layout.grid.x+9u,
                                                layout.grid.y+9u);
    CHECK(water!=0u);
    for(unsigned k=0;k<20u;++k) {
        human.offset=k*7u;
        human.n=0u;
        OK(warship_deploy_apply(&game,WARSHIP_DEPLOY_AUTO,
                               random_u32,&human,random_u32,&captain));
        EQ(human.n,5u);
        observe(&game,&pub);
        OK(warship_deploy_make_frame(&pub,&frame));
        count=0u;
        for(unsigned j=0;j<100u;++j) count+=frame.cells[j]!=0u;
        EQ(count,17u);
        EQ(pub.phase,WARSHIP_LIFECYCLE_DEPLOY);
    }
    const WarshipLifecycle saved=game;
    human.n=0u;human.fail_at=3u;
    CHECK(warship_deploy_apply(&game,WARSHIP_DEPLOY_AUTO,
                               random_u32,&human,random_u32,&captain)!=
          WARSHIP_DEPLOY_OK);
    same(&game,&saved,sizeof game);
    human.fail_at=0u;
    CHECK(warship_deploy_apply(&game,WARSHIP_DEPLOY_READY,
                               NULL,NULL,NULL,NULL)==WARSHIP_DEPLOY_OK);
    EQ(game.phase,WARSHIP_LIFECYCLE_COMBAT);
    observe(&game,&pub);
    OK(warship_deploy_make_frame(&pub,&frame));
    EQ(frame.mode,2u);
    CHECK(warship_deploy_apply(&game,WARSHIP_DEPLOY_RETURN,
                               NULL,NULL,NULL,NULL)==WARSHIP_DEPLOY_GAME_ERROR);
    CHECK(warship_deploy_apply(&game,WARSHIP_DEPLOY_AUTO,
                               random_u32,&human,NULL,NULL)==WARSHIP_DEPLOY_GAME_ERROR);
    CHECK(warship_deploy_apply(&game,WARSHIP_DEPLOY_READY,
                               NULL,NULL,NULL,NULL)==WARSHIP_DEPLOY_GAME_ERROR);
    EQ(game.phase,WARSHIP_LIFECYCLE_COMBAT);
    EQ(warship_deploy_hit(&layout,game.phase,270u,179u),WARSHIP_DEPLOY_NO_ACTION);

    WarshipLifecycle game2;
    CHECK(warship_lifecycle_init(&game2)==WARSHIP_LIFECYCLE_OK);
    human=(FakeRandom){0u,0u,0u}; captain=(FakeRandom){0u,10u,0u};
    OK(warship_deploy_apply(&game2,WARSHIP_DEPLOY_START,
                            random_u32,&human,random_u32,&captain));
    WarshipLifecyclePublic other;
    observe(&game2,&other);
    OK(warship_deploy_make_frame(&other,&frame));
    const WarshipFleet human_fleet=other.human;
    /* Change ONLY private captain fleet, keep same public human.
     * Raster and public frame stay identical. */
    captain=(FakeRandom){0u,40u,0u};
    WarshipLifecycle game3;
    CHECK(warship_lifecycle_init(&game3)==WARSHIP_LIFECYCLE_OK);
    human=(FakeRandom){0u,0u,0u};
    OK(warship_deploy_apply(&game3,WARSHIP_DEPLOY_START,
                            random_u32,&human,random_u32,&captain));
    WarshipLifecyclePublic other3;
    observe(&game3,&other3);
    same(&other3.human,&human_fleet,sizeof human_fleet);
    CHECK(memcmp(&game2.captain,&game3.captain,sizeof game2.captain)!=0);
    WarshipDeployFrame f2;
    OK(warship_deploy_make_frame(&other3,&f2));
    same(&frame,&f2,sizeof frame);

    OK(warship_deploy_apply(&game2,WARSHIP_DEPLOY_RETURN,
                            NULL,NULL,NULL,NULL));
    EQ(game2.phase,WARSHIP_LIFECYCLE_HOME);
    observe(&game2,&other);
    OK(warship_deploy_make_frame(&other,&frame));
    EQ(frame.mode,0u);
    OK(warship_deploy_apply(&game2,WARSHIP_DEPLOY_START,
                            random_u32,&human,random_u32,&captain));
    EQ(game2.phase,WARSHIP_LIFECYCLE_DEPLOY);
    /* No D07 terminal/forfeit event interface is called in this module. */

    WarshipDeployFrame invalid;
    memset(&invalid,0xc7,sizeof invalid);
    const WarshipDeployFrame before=invalid;
    other3.has_human_fleet=0u;
    EQ(warship_deploy_make_frame(&other3,&invalid),WARSHIP_DEPLOY_INVALID_FLEET);
    same(&invalid,&before,sizeof invalid);
    other3.has_human_fleet=1u;
    other3.human.ships[0].x=10u;
    EQ(warship_deploy_make_frame(&other3,&invalid),WARSHIP_DEPLOY_INVALID_FLEET);
    same(&invalid,&before,sizeof invalid);
}
int main(void)
{
    geometry();
    input();
    invalid_release_bounce();
    invalid_midpress_bounce();
    valid_short_release_bounce();
    lifecycle_and_frame();
    printf("D09 PASS sizeof(frame)=%zu sizeof(input)=%zu\n",
           sizeof(WarshipDeployFrame),sizeof(WarshipDeployInput));
    return EXIT_SUCCESS;
}
