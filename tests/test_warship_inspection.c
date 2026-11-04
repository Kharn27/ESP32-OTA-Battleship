#include "warship_inspection.h"
#include "warship_journey.h"
#include "warship_palmares.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do {if(!(x)){fprintf(stderr,"D13 FAIL %s:%d %s\n",__FILE__,__LINE__,#x);exit(EXIT_FAILURE);}}while(0)
#define EQ(a,b) CHECK((a)==(b))
#define LOK(a) EQ((a),WARSHIP_LIFECYCLE_OK)
#define UOK(a) EQ((a),WARSHIP_COMBAT_OK)
#define IOK(a) EQ((a),WARSHIP_INSPECTION_APPLIED)
#define JOK(a) EQ((a),WARSHIP_JOURNEY_DONE)

static WarshipLifecycle game;
static WarshipLifecyclePublic observation;
static WarshipCombatUi ui;
static WarshipCombatInput input;
static WarshipCombatFrame frame;

typedef struct {uint32_t values[5];unsigned calls;int fail;} FleetRng;
static int fleet_rng(void *ctx,uint32_t *out)
{
    FleetRng *r=(FleetRng *)ctx;
    if(r->fail || out==NULL)return -1;
    *out=r->values[r->calls++%5u];
    return 0;
}
static FleetRng human_rng(void){FleetRng r={{0u,20u,40u,60u,80u},0u,0};return r;}
static FleetRng captain_rng(void){FleetRng r={{10u,30u,50u,70u,90u},0u,0};return r;}
static FleetRng replay_rng(void){FleetRng r={{12u,32u,52u,72u,92u},0u,0};return r;}
static int cpu_rng(void *ctx,uint32_t *out){(void)ctx;*out=0u;return 0;}
static WarshipEventStatus no_score(void *ctx,WarshipLifecycleEvent event)
{
    (void)ctx;(void)event;CHECK(0);return WARSHIP_EVENT_CONFLICT;
}
static int no_score_load(void *ctx,uint8_t *d,size_t cap,size_t *actual)
{
    (void)ctx;(void)d;(void)cap;(void)actual;return 1;
}
static unsigned score_writes;
static int score_store(void *ctx,const uint8_t *data,size_t len)
{
    (void)ctx;(void)data;(void)len;++score_writes;return 0;
}
static WarshipPalmares ledger;
static void observe(void){LOK(warship_lifecycle_observe(&game,&observation));}
static void start(void)
{
    FleetRng human=human_rng(),captain=captain_rng();
    LOK(warship_lifecycle_init(&game));
    LOK(warship_lifecycle_start(&game,fleet_rng,&human,
                               fleet_rng,&captain));
    LOK(warship_lifecycle_ready(&game));
    warship_combat_init(&ui,&input,0u);
    warship_inspection_clear_frame(&frame);
    observe();
    EQ(observation.can_reveal_captain,0u);
    EQ(observation.reveal_visible,0u);
    WarshipPalmaresBackend b={no_score_load,score_store};
    EQ(warship_palmares_open(&ledger,b,NULL),WARSHIP_PALMARES_APPLIED);
    score_writes=0u;
}
static void ship_cell(unsigned ordinal,uint8_t *x,uint8_t *y)
{
    static const uint8_t lengths[5]={2u,3u,3u,4u,5u};
    for(unsigned i=0u;i<5u;++i){
        if(ordinal<lengths[i]){
            *x=(uint8_t)(5u+ordinal);*y=(uint8_t)i;return;
        }
        ordinal-=lengths[i];
    }
    CHECK(0);
}
static int occupied(unsigned x,unsigned y)
{
    static const uint8_t lengths[5]={2u,3u,3u,4u,5u};
    return y<5u&&x>=5u&&x<5u+lengths[y];
}
static void water_cell(unsigned ordinal,uint8_t *x,uint8_t *y)
{
    for(unsigned p=0u;p<100u;++p){
        if(occupied(p%10u,p/10u))continue;
        if(ordinal==0u){*x=(uint8_t)(p%10u);*y=(uint8_t)(p/10u);return;}
        --ordinal;
    }
    CHECK(0);
}
static void select_and_fire(uint8_t x,uint8_t y,unsigned ms)
{
    observe();
    WarshipCombatHit cell={WARSHIP_COMBAT_CELL,x,y};
    UOK(warship_combat_apply(&ui,&game,&observation,
                             cell,ms,cpu_rng,NULL));
    observe();
    WarshipCombatHit fire={WARSHIP_COMBAT_FIRE,0u,0u};
    UOK(warship_combat_apply(&ui,&game,&observation,
                             fire,ms+1u,cpu_rng,NULL));
    CHECK(ui.pending);
    observe();
    UOK(warship_combat_make_frame(&observation,&ui,0u,&frame));
    IOK(warship_inspection_overlay(&game,&observation,&ui,&frame));
    for(unsigned i=0u;i<100u;++i)EQ(frame.ships[i],0u);
    EQ(frame.reveal_available,0u);
    UOK(warship_combat_make_frame(&observation,&ui,1u,&frame));
    IOK(warship_inspection_overlay(&game,&observation,&ui,&frame));
    for(unsigned i=0u;i<100u;++i)EQ(frame.ships[i],0u);
}
static WarshipLifecycleEvent finish(unsigned mode,unsigned draw_at)
{
    /* Reproducible real D07+D06+D02 history: only injected fleet RNG
     * and Captain *public* target RNG. No private fleet reads or forged
     * result; D07 alone emits the terminal event AFTER visual ACK. */
    WarshipLifecycleEvent terminal=WARSHIP_LIFECYCLE_EVENT_NONE;
    for(unsigned n=0u;n<100u;++n){
        uint8_t x=0u,y=0u;
        if(mode==0u||n<16u||(mode==2u&&n+1u==draw_at))
            ship_cell(n<16u?n:16u,&x,&y);
        else water_cell(n-16u,&x,&y);
        select_and_fire(x,y,n*300u);
        WarshipLifecycleEvent e=WARSHIP_LIFECYCLE_EVENT_NONE;
        UOK(warship_combat_presented(&ui,&game,&e));
        CHECK(!ui.pending);
        if(e!=WARSHIP_LIFECYCLE_EVENT_NONE){
            terminal=e;
            break;
        }
    }
    EQ(game.phase,WARSHIP_LIFECYCLE_RESULT);
    EQ(terminal,mode==0u?WARSHIP_LIFECYCLE_EVENT_VICTORY:
       mode==1u?WARSHIP_LIFECYCLE_EVENT_DEFEAT:
       WARSHIP_LIFECYCLE_EVENT_DRAW);
    observe();
    CHECK(observation.has_battle);
    EQ(observation.battle.history.verdict,
       mode==0u?WARSHIP_BATTLE_VICTORY:
       mode==1u?WARSHIP_BATTLE_DEFEAT:WARSHIP_BATTLE_DRAW);
    EQ(observation.reveal_visible,0u);
    EQ(observation.reveal_requested,0u);
    EQ(warship_palmares_apply(&ledger,1u,terminal),
       WARSHIP_PALMARES_APPLIED);
    EQ(score_writes,1u);
    return terminal;
}
static WarshipCombatHit inspect(void)
{
    WarshipCombatHit h=warship_combat_hit(WARSHIP_COMBAT_ATTACK,270u,220u);
    EQ(h.command,WARSHIP_COMBAT_INSPECT);
    return h;
}
static void project(void)
{
    observe();
    UOK(warship_combat_make_frame(&observation,&ui,0u,&frame));
    IOK(warship_inspection_overlay(&game,&observation,&ui,&frame));
}
static unsigned count_ships(void)
{
    unsigned n=0u;
    for(unsigned i=0u;i<100u;++i)n+=(frame.ships[i]!=0u);
    return n;
}
static void assert_ledger_unchanged(void)
{
    WarshipPalmaresView v;
    EQ(warship_palmares_read(&ledger,&v),WARSHIP_PALMARES_APPLIED);
    EQ(v.games,1u);EQ(v.voluntary_forfeits,0u);
    EQ(v.victories+v.defeats+v.draws,1u);
    EQ(score_writes,1u);
}
static void assert_no_secret(void)
{
    project();
    EQ(count_ships(),ui.view==WARSHIP_COMBAT_ATTACK?0u:17u);
    if(ui.view==WARSHIP_COMBAT_ATTACK) {
        WarshipFleet leak;
        memset(&leak,0xa5,sizeof leak);
        WarshipFleet before=leak;
        EQ(warship_lifecycle_copy_revealed_captain(&game,&leak),
           WARSHIP_LIFECYCLE_REVEAL_DENIED);
        CHECK(memcmp(&leak,&before,sizeof leak)==0);
    }
}
static void inspect_defeat(void)
{
    start();finish(1u,0u);
    EQ(observation.can_reveal_captain,1u);
    const WarshipSoloSession original=game.solo;
    const uint8_t verdict=game.verdict;
    assert_no_secret();
    WarshipCombatFrame original_frame=frame;
    const WarshipCombatHit h=inspect();
    CHECK(warship_combat_pixel(&frame,270u,220u)!=0u);

    /* Exact D10 D12 input contract: 40ms stable DOWN, 60ms UP and
     * 120ms inter-command, no double action on long hold. */
    WarshipCombatHit out;
    UOK(warship_journey_input_step(&input,&ui,0u,1u,0u,0u,60000u,&out));
    UOK(warship_journey_input_step(&input,&ui,0u,1u,0u,0u,60060u,&out));
    UOK(warship_journey_input_step(&input,&ui,1u,1u,270u,220u,60100u,&out));
    EQ(out.command,WARSHIP_COMBAT_NO_HIT);
    UOK(warship_journey_input_step(&input,&ui,1u,1u,270u,220u,60140u,&out));
    EQ(out.command,WARSHIP_COMBAT_INSPECT);
    IOK(warship_inspection_toggle(&game,&observation,&ui,out));
    warship_inspection_clear_frame(&frame);
    UOK(warship_journey_input_step(&input,&ui,1u,1u,270u,220u,60220u,&out));
    EQ(out.command,WARSHIP_COMBAT_NO_HIT);
    warship_combat_input_discard(&input);
    UOK(warship_journey_input_step(&input,&ui,1u,1u,270u,220u,60300u,&out));
    EQ(out.command,WARSHIP_COMBAT_NO_HIT);

    project();
    EQ(frame.reveal_available,1u);EQ(frame.reveal_requested,1u);
    EQ(frame.reveal_visible,1u);EQ(count_ships(),17u);
    EQ(frame.phase,WARSHIP_LIFECYCLE_RESULT);
    const WarshipCombatFrame revealed_frame=frame;
    /* Expected fleet comes independently from the public test fixture,
     * never from game.captain or the D06 private session. */
    WarshipFleet expected;
    FleetRng cpu=captain_rng();
    EQ(warship_fleet_generate(&expected,fleet_rng,&cpu),WARSHIP_FLEET_OK);
    for(unsigned i=0u;i<WARSHIP_FLEET_SHIPS;++i){
        WarshipShip ship=expected.ships[i];
        unsigned occupied_count=0u;
        for(unsigned k=0u;k<100u;++k)occupied_count+=frame.ships[k]==i+1u;
        EQ(occupied_count,ship.length);
    }
    for(unsigned i=0u;i<100u;++i)
        EQ(frame.attacks[i],original_frame.attacks[i]);
    CHECK(memcmp(&original,&game.solo,sizeof original)==0);
    EQ(verdict,game.verdict);
    assert_ledger_unchanged();

    /* Touch/HIT/SUNK marks override the new ship backing pixels. */
    unsigned tested_overlap=0u;
    for(unsigned y=0u;y<10u;++y)for(unsigned x=0u;x<10u;++x) {
        const unsigned k=y*10u+x;
        if(frame.ships[k] && frame.attacks[k]>=2u) {
            const uint16_t cx=(uint16_t)(22u+x*19u+4u);
            const uint16_t cy=(uint16_t)(24u+y*19u+4u);
            const uint16_t before=warship_combat_pixel(&original_frame,cx,cy);
            const uint16_t after=warship_combat_pixel(&frame,cx,cy);
            EQ(after,before);
            ++tested_overlap;
        }
    }
    CHECK(tested_overlap>0u);

    /* DEFENSE must show only human fleet, keep D07 reveal flag.
     * Return to ATTAQUE reconstitutes overlay, no secret cache retained
     * in frame during DEFENSE. */
    WarshipCombatHit sw={WARSHIP_COMBAT_SWITCH,0u,0u};
    UOK(warship_combat_apply(&ui,&game,&observation,sw,70000u,cpu_rng,NULL));
    project();
    EQ(ui.view,WARSHIP_COMBAT_DEFENSE);
    EQ(count_ships(),17u);
    EQ(observation.reveal_visible,1u);
    for(unsigned i=0u;i<100u;++i) {
        const WarshipSoloPublic *history=&observation.battle;
        uint8_t expected=0u;
        for(unsigned j=0u;j<history->history.incoming_count;++j) {
            const WarshipBattleShot hit=history->history.incoming[j];
            if(hit.y*10u+hit.x==i) expected=(uint8_t)(hit.impact+1u);
        }
        EQ(frame.attacks[i],expected);
    }
    EQ(warship_inspection_toggle(&game,&observation,&ui,h),WARSHIP_INSPECTION_IGNORED);
    UOK(warship_combat_apply(&ui,&game,&observation,sw,70100u,cpu_rng,NULL));
    project();
    EQ(count_ships(),17u);EQ(ui.view,WARSHIP_COMBAT_ATTACK);
    for(unsigned i=0u;i<100u;++i) {
        EQ(frame.attacks[i],original_frame.attacks[i]);
        EQ(frame.ships[i],revealed_frame.ships[i]);
    }

    /* HIDE purges the presentation copy IMMEDIATELY and does not
     * revoke permission. No CPU pixels accessible after frame clear. */
    IOK(warship_inspection_toggle(&game,&observation,&ui,h));
    warship_inspection_clear_frame(&frame);
    for(unsigned i=0u;i<100u;++i)
        EQ(frame.ships[i],0u);
    assert_no_secret();
    EQ(observation.reveal_requested,1u);
    EQ(observation.reveal_visible,0u);
    EQ(frame.reveal_available,1u);
    assert_ledger_unchanged();

    /* SHOW again: no new game/score; then result->REJOUER must purge. */
    IOK(warship_inspection_toggle(&game,&observation,&ui,h));
    project();EQ(count_ships(),17u);
    WarshipEventStatus persist=WARSHIP_EVENT_PENDING;
    FleetRng next=replay_rng();
    WarshipCombatHit play={WARSHIP_COMBAT_FIRE,0u,0u};
    JOK(warship_journey_apply(&game,&ui,play,fleet_rng,&next,
                             no_score,NULL,&persist));
    warship_inspection_clear_frame(&frame);
    EQ(game.phase,WARSHIP_LIFECYCLE_DEPLOY);
    EQ(persist,WARSHIP_EVENT_NO_CHANGE);
    for(unsigned i=0u;i<100u;++i)EQ(frame.ships[i],0u);
    observe();EQ(observation.can_reveal_captain,0u);
    EQ(observation.reveal_requested,0u);EQ(observation.reveal_visible,0u);
    EQ(warship_inspection_toggle(&game,&observation,&ui,h),WARSHIP_INSPECTION_IGNORED);
    EQ(warship_lifecycle_copy_revealed_captain(&game,&expected),
       WARSHIP_LIFECYCLE_REVEAL_DENIED);
    assert_ledger_unchanged();
    LOK(warship_lifecycle_leave_deploy(&game));
    EQ(game.phase,WARSHIP_LIFECYCLE_HOME);
    EQ(warship_inspection_toggle(&game,&observation,&ui,h),WARSHIP_INSPECTION_IGNORED);
    assert_ledger_unchanged();
    /* A fresh game cannot inherit an opt-in reveal or stale frame. */
    LOK(warship_lifecycle_init(&game));
    observe();EQ(observation.reveal_visible,0u);
}
static void inspect_other_verdicts(unsigned mode,unsigned draw_at)
{
    start();finish(mode,draw_at);
    CHECK(!observation.can_reveal_captain);
    assert_no_secret();
    const WarshipLifecycle original=game;
    const WarshipCombatUi before_ui=ui;
    const WarshipCombatHit h=inspect();
    EQ(warship_inspection_toggle(&game,&observation,&ui,h),WARSHIP_INSPECTION_IGNORED);
    CHECK(memcmp(&original,&game,sizeof game)==0);
    CHECK(memcmp(&before_ui,&ui,sizeof ui)==0);
    WarshipFleet leak;
    memset(&leak,0xf3,sizeof leak);
    const WarshipFleet before=leak;
    EQ(warship_lifecycle_copy_revealed_captain(&game,&leak),
       WARSHIP_LIFECYCLE_REVEAL_DENIED);
    CHECK(memcmp(&leak,&before,sizeof leak)==0);
    WarshipCombatHit fire={WARSHIP_COMBAT_FIRE,0u,0u};
    EQ(warship_combat_apply(&ui,&game,&observation,fire,1000u,cpu_rng,NULL),
       WARSHIP_COMBAT_IGNORED);
    WarshipCombatHit cell={WARSHIP_COMBAT_CELL,1u,1u};
    EQ(warship_combat_apply(&ui,&game,&observation,cell,1001u,cpu_rng,NULL),
       WARSHIP_COMBAT_IGNORED);
    CHECK(memcmp(&original,&game,sizeof game)==0);
    /* Inspect both histories and leave result without any extra score. */
    WarshipCombatHit sw={WARSHIP_COMBAT_SWITCH,0u,0u};
    UOK(warship_combat_apply(&ui,&game,&observation,sw,1010u,cpu_rng,NULL));
    project();EQ(count_ships(),17u);
    UOK(warship_combat_apply(&ui,&game,&observation,sw,1020u,cpu_rng,NULL));
    assert_no_secret();
    WarshipEventStatus persisted=WARSHIP_EVENT_PENDING;
    WarshipCombatHit home={WARSHIP_COMBAT_CANCEL,0u,0u};
    JOK(warship_journey_apply(&game,&ui,home,NULL,NULL,
                             no_score,NULL,&persisted));
    warship_inspection_clear_frame(&frame);
    EQ(game.phase,WARSHIP_LIFECYCLE_HOME);
    EQ(persisted,WARSHIP_EVENT_NO_CHANGE);
    assert_ledger_unchanged();
}
static void invalid_guards(void)
{
    start();
    WarshipCombatHit h=inspect();
    EQ(warship_inspection_toggle(&game,&observation,&ui,h),WARSHIP_INSPECTION_IGNORED);
    WarshipInspectionStatus s=warship_inspection_toggle(NULL,&observation,&ui,h);
    EQ(s,WARSHIP_INSPECTION_BAD_ARGUMENT);
    s=warship_inspection_toggle(&game,&observation,NULL,h);
    EQ(s,WARSHIP_INSPECTION_BAD_ARGUMENT);
    s=warship_inspection_toggle(&game,NULL,&ui,h);
    EQ(s,WARSHIP_INSPECTION_BAD_ARGUMENT);
    h.command=255u;
    EQ(warship_inspection_toggle(&game,&observation,&ui,h),
       WARSHIP_INSPECTION_BAD_ARGUMENT);
    EQ(warship_inspection_overlay(NULL,&observation,&ui,&frame),
       WARSHIP_INSPECTION_BAD_ARGUMENT);
    EQ(warship_inspection_overlay(&game,NULL,&ui,&frame),
       WARSHIP_INSPECTION_BAD_ARGUMENT);
    EQ(warship_inspection_overlay(&game,&observation,NULL,&frame),
       WARSHIP_INSPECTION_BAD_ARGUMENT);
    EQ(warship_inspection_overlay(&game,&observation,&ui,NULL),
       WARSHIP_INSPECTION_BAD_ARGUMENT);
    EQ(warship_combat_hit(WARSHIP_COMBAT_ATTACK,226u,220u).command,
       WARSHIP_COMBAT_NO_HIT);
    EQ(warship_combat_hit(WARSHIP_COMBAT_ATTACK,313u,231u).command,
       WARSHIP_COMBAT_INSPECT);
    EQ(warship_combat_hit(WARSHIP_COMBAT_ATTACK,314u,231u).command,
       WARSHIP_COMBAT_NO_HIT);
    EQ(warship_combat_hit(WARSHIP_COMBAT_ATTACK,270u,234u).command,
       WARSHIP_COMBAT_NO_HIT);
    WarshipCombatHit t={WARSHIP_COMBAT_INSPECT,0u,0u};
    EQ(warship_inspection_toggle(&game,&observation,&ui,t),WARSHIP_INSPECTION_IGNORED);
    observe();
    const WarshipLifecycle before=game;
    EQ(warship_combat_apply(&ui,&game,&observation,t,1u,cpu_rng,NULL),
       WARSHIP_COMBAT_IGNORED);
    WarshipEventStatus untouched=WARSHIP_EVENT_NO_CHANGE;
    EQ(warship_journey_apply(&game,&ui,t,NULL,NULL,no_score,NULL,
                            &untouched),WARSHIP_JOURNEY_NOT_HANDLED);
    EQ(untouched,WARSHIP_EVENT_NO_CHANGE);
    EQ(warship_lifecycle_set_captain_visible(&game,1u),
       WARSHIP_LIFECYCLE_REVEAL_DENIED);
    CHECK(memcmp(&before,&game,sizeof game)==0);
}
int main(void)
{
    invalid_guards();
    inspect_defeat();
    inspect_other_verdicts(0u,0u);
    /* D11's deterministic loss length serves as DRAW turn fixture.
     * Use the same public CPU RNG and 16 hit + water choices as loss. */
    start();
    unsigned loss_at=0u;
    for(unsigned n=0u;n<100u;++n){
        uint8_t x=0u,y=0u;
        if(n<16u)ship_cell(n,&x,&y);
        else water_cell(n-16u,&x,&y);
        select_and_fire(x,y,n*300u);
        WarshipLifecycleEvent e=WARSHIP_LIFECYCLE_EVENT_NONE;
        UOK(warship_combat_presented(&ui,&game,&e));
        if(e==WARSHIP_LIFECYCLE_EVENT_DEFEAT){loss_at=n+1u;break;}
    }
    CHECK(loss_at>=17u && loss_at<100u);
    inspect_other_verdicts(2u,loss_at);
    puts("D13 real D07/D10/D12 inspection C99 PASS");
    return EXIT_SUCCESS;
}
