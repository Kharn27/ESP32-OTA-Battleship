#include "warship_combat_ui.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static WarshipLifecycle game;
static WarshipLifecyclePublic pub;
static WarshipCombatUi ui;
static WarshipCombatInput in;
static WarshipCombatFrame frame;
static uint32_t human_rng=0x12345678u, cpu_fleet_rng=0x87654321u;
static uint32_t attack_rng=0x13572468u;

static int random_fail(void *ctx, uint32_t *out)
{
    (void)ctx; (void)out;
    return -1;
}
static int random_u32(void *ctx, uint32_t *out)
{
    uint32_t *seed=(uint32_t *)ctx;
    if (!seed || !out) return -1;
    *seed ^= *seed << 13;
    *seed ^= *seed >> 17;
    *seed ^= *seed << 5;
    *out = *seed;
    return 0;
}
static void observe(void)
{
    assert(warship_lifecycle_observe(&game,&pub)==WARSHIP_LIFECYCLE_OK);
}
static WarshipCombatHit hit(uint8_t type,uint8_t x,uint8_t y)
{
    WarshipCombatHit h={type,x,y};
    return h;
}
static WarshipCombatStatus act(uint8_t type,uint8_t x,uint8_t y,unsigned ms)
{
    return warship_combat_apply(&ui,&game,&pub,hit(type,x,y),ms,
                                random_u32,&attack_rng);
}
static void input_tests(void)
{
    WarshipCombatInput g;
    WarshipCombatHit h;
    warship_combat_init(NULL,&g,0u);
    assert(warship_combat_input_step(&g,1,1,22,24,40,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(h.command==WARSHIP_COMBAT_NO_HIT); /* READY press */
    assert(warship_combat_input_step(&g,0,1,0,0,50,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,0,1,0,0,110,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,1,1,22,24,150,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(h.command==WARSHIP_COMBAT_NO_HIT);
    assert(warship_combat_input_step(&g,1,1,22,24,190,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(h.command==WARSHIP_COMBAT_CELL && h.x==0 && h.y==0);
    assert(warship_combat_input_step(&g,1,1,225,50,280,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(h.command==WARSHIP_COMBAT_NO_HIT); /* holding cannot FIRE */
    assert(warship_combat_input_step(&g,0,1,0,0,290,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,1,1,230,40,320,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,1,1,230,40,360,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(h.command==WARSHIP_COMBAT_NO_HIT); /* UP bounce < 60 ms */
    assert(warship_combat_input_step(&g,0,1,0,0,370,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,0,1,0,0,430,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,1,1,240,55,460,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,1,1,240,55,500,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(h.command==WARSHIP_COMBAT_FIRE);
    assert(warship_combat_input_step(&g,0,1,0,0,510,1,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,0,1,0,0,570,1,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,1,1,240,98,600,1,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,1,1,240,98,650,1,0,&h)==WARSHIP_COMBAT_OK);
    assert(h.command==WARSHIP_COMBAT_NO_HIT); /* animation drops DEFENSE */
    assert(warship_combat_input_step(&g,1,1,240,98,700,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(h.command==WARSHIP_COMBAT_NO_HIT); /* no deferred switch */
    assert(warship_combat_input_step(&g,0,1,0,0,710,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,0,1,0,0,770,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,1,1,240,98,800,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,1,1,240,98,840,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(h.command==WARSHIP_COMBAT_SWITCH);
    warship_combat_input_discard(&g);
    assert(!g.armed && g.consumed);
    assert(warship_combat_input_step(&g,0,1,0,0,850,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,0,1,0,0,910,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,1,0,240,98,940,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(warship_combat_input_step(&g,1,1,240,98,990,0,0,&h)==WARSHIP_COMBAT_OK);
    assert(h.command==WARSHIP_COMBAT_NO_HIT); /* invalid first sample */
    const WarshipCombatInput saved=g;
    h.command=WARSHIP_COMBAT_FIRE;
    assert(warship_combat_input_step(&g,1,1,240,98,930,0,0,&h)==WARSHIP_COMBAT_BAD_ARGUMENT);
    assert(memcmp(&saved,&g,sizeof g)==0 && h.command==WARSHIP_COMBAT_FIRE);
}
static void layout_tests(void)
{
    WarshipCombatHit h=warship_combat_hit(0,22,24);
    assert(h.command==WARSHIP_COMBAT_CELL && h.x==0 && h.y==0);
    h=warship_combat_hit(0,211,213);
    assert(h.command==WARSHIP_COMBAT_CELL && h.x==9 && h.y==9);
    assert(warship_combat_hit(0,212,213).command==WARSHIP_COMBAT_NO_HIT);
    assert(warship_combat_hit(0,211,214).command==WARSHIP_COMBAT_NO_HIT);
    assert(warship_combat_hit(1,211,213).command==WARSHIP_COMBAT_NO_HIT);
    assert(warship_combat_hit(0,230,40).command==WARSHIP_COMBAT_FIRE);
    assert(warship_combat_hit(1,230,40).command==WARSHIP_COMBAT_NO_HIT);
    assert(warship_combat_hit(1,230,91).command==WARSHIP_COMBAT_SWITCH);
    assert(warship_combat_hit(0,230,150).command==WARSHIP_COMBAT_CANCEL);
    assert(warship_combat_hit(0,320,120).command==WARSHIP_COMBAT_NO_HIT);
    assert(warship_combat_max_alert(0,WARSHIP_BATTLE_HIT)==2);
    assert(warship_combat_max_alert(2,WARSHIP_BATTLE_SUNK)==3);
    assert(warship_combat_max_alert(3,WARSHIP_BATTLE_MISS)==3);
}
static void start(void)
{
    assert(warship_lifecycle_init(&game)==WARSHIP_LIFECYCLE_OK);
    assert(warship_lifecycle_start(&game,random_u32,&human_rng,
            random_u32,&cpu_fleet_rng)==WARSHIP_LIFECYCLE_OK);
    observe();
    assert(pub.phase==WARSHIP_LIFECYCLE_DEPLOY);
    assert(warship_lifecycle_ready(&game)==WARSHIP_LIFECYCLE_OK);
    warship_combat_init(&ui,&in,1000u);
    observe();
    assert(pub.phase==WARSHIP_LIFECYCLE_COMBAT && pub.has_battle);
}
static void combat_tests(void)
{
    assert(warship_combat_make_frame(&pub,&ui,0,&frame)==WARSHIP_COMBAT_OK);
    assert(frame.view==WARSHIP_COMBAT_ATTACK && !frame.has_selection);
    assert(!frame.has_cpu && !frame.has_human && !frame.terminal);
    for (unsigned i=0;i<100;i++)
        assert(frame.ships[i]==0 && frame.attacks[i]==0);
    assert(warship_combat_pixel(&frame,22,24)!=0u);
    assert(act(WARSHIP_COMBAT_FIRE,0,0,1100)==WARSHIP_COMBAT_IGNORED);
    assert(!ui.pending);
    assert(act(WARSHIP_COMBAT_CELL,0,0,1200)==WARSHIP_COMBAT_OK);
    observe();
    assert(pub.battle.has_selection && pub.battle.target_x==0);
    assert(act(WARSHIP_COMBAT_CELL,0,0,1300)==WARSHIP_COMBAT_OK);
    observe();
    assert(!pub.battle.has_selection); /* same target toggles off */
    assert(act(WARSHIP_COMBAT_CELL,0,0,1400)==WARSHIP_COMBAT_OK);
    observe();
    assert(act(WARSHIP_COMBAT_CELL,1,1,1500)==WARSHIP_COMBAT_OK);
    observe();
    assert(pub.battle.target_x==1 && pub.battle.target_y==1);
    assert(act(WARSHIP_COMBAT_SWITCH,0,0,1600)==WARSHIP_COMBAT_OK);
    observe();
    assert(warship_combat_make_frame(&pub,&ui,0,&frame)==WARSHIP_COMBAT_OK);
    unsigned occupied=0;
    for (unsigned i=0;i<100;i++) occupied+=(frame.ships[i]!=0u);
    assert(occupied==17u); /* human fleet only, DEFENSE */
    assert(act(WARSHIP_COMBAT_SWITCH,0,0,1700)==WARSHIP_COMBAT_OK);
    observe();
    assert(pub.battle.has_selection && pub.battle.target_x==1 &&
           pub.battle.target_y==1);
    assert(warship_combat_make_frame(&pub,&ui,0,&frame)==WARSHIP_COMBAT_OK);
    for (unsigned i=0;i<100;i++) assert(frame.ships[i]==0u);
    assert(frame.has_selection && frame.target_x==1 && frame.target_y==1);
    /* RNG failure cannot advance D07, the private session or UI. */
    static WarshipLifecycle before;
    before=game;
    const WarshipCombatUi before_ui=ui;
    assert(warship_combat_apply(&ui,&game,&pub,hit(WARSHIP_COMBAT_FIRE,0,0),
                                1790u,random_fail,NULL)==WARSHIP_COMBAT_GAME_ERROR);
    assert(memcmp(&before,&game,sizeof game)==0);
    assert(memcmp(&before_ui,&ui,sizeof ui)==0);
    assert(act(WARSHIP_COMBAT_FIRE,0,0,1800)==WARSHIP_COMBAT_OK);
    assert(ui.pending && ui.view==WARSHIP_COMBAT_ATTACK && !ui.reveals);
    observe();
    assert(pub.battle.phase==WARSHIP_SOLO_AWAIT_REVEAL);
    assert(pub.battle.history.outgoing_count==0);
    assert(pub.battle.history.incoming_count==0);
    assert(warship_combat_make_frame(&pub,&ui,0,&frame)==WARSHIP_COMBAT_OK);
    assert(frame.busy && !frame.has_cpu && !frame.has_human);
    assert(frame.attacks[11]==0u);
    assert(warship_combat_make_frame(&pub,&ui,1,&frame)==WARSHIP_COMBAT_OK);
    assert(!frame.busy && frame.has_cpu && frame.has_human);
    assert(frame.attacks[11]==ui.pending_pair.human.impact+1u);
    assert(frame.last_cpu.x==ui.pending_pair.captain.x);
    assert(frame.last_cpu.y==ui.pending_pair.captain.y);
    for (unsigned i=0;i<100;i++) assert(frame.ships[i]==0);
    assert(!warship_combat_reveal_due(&ui,1979));
    assert(warship_combat_reveal_due(&ui,1980));
    assert(act(WARSHIP_COMBAT_SWITCH,0,0,1980)==WARSHIP_COMBAT_IGNORED);
    assert(act(WARSHIP_COMBAT_FIRE,0,0,1980)==WARSHIP_COMBAT_IGNORED);
    assert(act(WARSHIP_COMBAT_CELL,2,2,1980)==WARSHIP_COMBAT_IGNORED);
    assert(act(WARSHIP_COMBAT_CANCEL,0,0,1980)==WARSHIP_COMBAT_IGNORED);
    assert(ui.view==WARSHIP_COMBAT_ATTACK && ui.pending_pair.round==1u);
    WarshipLifecycleEvent event=WARSHIP_LIFECYCLE_EVENT_NONE;
    assert(warship_combat_presented(&ui,&game,&event)==WARSHIP_COMBAT_OK);
    assert(!ui.pending && ui.reveals==1);
    assert(warship_combat_presented(&ui,&game,&event)==WARSHIP_COMBAT_BAD_ARGUMENT);
    observe();
    assert(pub.battle.history.outgoing_count==1);
    assert(pub.battle.history.incoming_count==1);
    assert(act(WARSHIP_COMBAT_CELL,1,1,2000)==WARSHIP_COMBAT_IGNORED);
    /* A previously attacked square cannot replace a valid new target. */
    assert(act(WARSHIP_COMBAT_CELL,2,2,2010)==WARSHIP_COMBAT_OK);
    observe();
    assert(pub.battle.has_selection && pub.battle.target_x==2 &&
           pub.battle.target_y==2);
    assert(act(WARSHIP_COMBAT_CELL,1,1,2020)==WARSHIP_COMBAT_IGNORED);
    observe();
    assert(pub.battle.has_selection && pub.battle.target_x==2 &&
           pub.battle.target_y==2);
    assert(act(WARSHIP_COMBAT_CANCEL,0,0,2030)==WARSHIP_COMBAT_OK);
    observe();
    assert(!pub.battle.has_selection);
    assert(act(WARSHIP_COMBAT_SWITCH,0,0,2100)==WARSHIP_COMBAT_OK);
    assert(!ui.has_summary && !ui.alert);
    observe();
    assert(warship_combat_make_frame(&pub,&ui,0,&frame)==WARSHIP_COMBAT_OK);
    assert(frame.view==WARSHIP_COMBAT_DEFENSE);
    assert(frame.has_cpu && frame.last_cpu.x==ui.last_cpu.x &&
           frame.last_cpu.y==ui.last_cpu.y);
    assert(frame.attacks[ui.last_cpu.y*10u+ui.last_cpu.x]!=0u);
    assert(act(WARSHIP_COMBAT_SWITCH,0,0,2200)==WARSHIP_COMBAT_OK);
    assert(!ui.has_summary && !ui.alert);
    /* Pending-severity maximum is independent of most recent shot. */
    ui.alert=warship_combat_max_alert(ui.alert,WARSHIP_BATTLE_HIT);
    ui.alert=warship_combat_max_alert(ui.alert,WARSHIP_BATTLE_SUNK);
    ui.alert=warship_combat_max_alert(ui.alert,WARSHIP_BATTLE_MISS);
    ui.last_cpu.impact=WARSHIP_BATTLE_MISS;
    ui.last_cpu.ship_id=WARSHIP_BATTLE_NO_SHIP;
    ui.has_summary=1u;
    observe();
    assert(warship_combat_make_frame(&pub,&ui,0,&frame)==WARSHIP_COMBAT_OK);
    assert(frame.alert==3u && frame.last_cpu.impact==WARSHIP_BATTLE_MISS);
    assert(act(WARSHIP_COMBAT_SWITCH,0,0,2300)==WARSHIP_COMBAT_OK);
    assert(ui.alert==0 && !ui.has_summary);
    assert(act(WARSHIP_COMBAT_SWITCH,0,0,2400)==WARSHIP_COMBAT_OK);

    /* Complete a whole deterministic real D02/D06/D07 match; one event. */
    for (unsigned i=0;i<100u && game.phase==WARSHIP_LIFECYCLE_COMBAT;i++) {
        if (i==11u) continue; /* already fired B2 */
        observe();
        assert(act(WARSHIP_COMBAT_CELL,(uint8_t)(i%10u),
                   (uint8_t)(i/10u),3000u+i*300u)==WARSHIP_COMBAT_OK);
        observe();
        assert(act(WARSHIP_COMBAT_FIRE,0,0,3010u+i*300u)==WARSHIP_COMBAT_OK);
        assert(ui.pending && ui.pending_pair.round==ui.reveals+1u);
        observe();
        assert(warship_combat_make_frame(&pub,&ui,1,&frame)==WARSHIP_COMBAT_OK);
        const unsigned k=ui.pending_pair.human.y*10u+ui.pending_pair.human.x;
        assert(frame.attacks[k]!=0u && frame.has_cpu);
        assert(warship_combat_presented(&ui,&game,&event)==WARSHIP_COMBAT_OK);
        assert(!ui.pending);
    }
    assert(game.phase==WARSHIP_LIFECYCLE_RESULT);
    assert(event==WARSHIP_LIFECYCLE_EVENT_VICTORY ||
           event==WARSHIP_LIFECYCLE_EVENT_DEFEAT ||
           event==WARSHIP_LIFECYCLE_EVENT_DRAW);
    assert(ui.terminal_event==(uint8_t)event);
    observe();
    assert(warship_combat_make_frame(&pub,&ui,0,&frame)==WARSHIP_COMBAT_OK);
    assert(frame.terminal && frame.verdict!=WARSHIP_BATTLE_CONTINUE);
    assert(act(WARSHIP_COMBAT_CELL,9,9,90000)==WARSHIP_COMBAT_IGNORED);
    assert(act(WARSHIP_COMBAT_FIRE,0,0,90001)==WARSHIP_COMBAT_IGNORED);
    assert(act(WARSHIP_COMBAT_SWITCH,0,0,90002)==WARSHIP_COMBAT_OK);
    observe();
    assert(warship_combat_make_frame(&pub,&ui,0,&frame)==WARSHIP_COMBAT_OK);
    assert(frame.terminal && frame.view==WARSHIP_COMBAT_DEFENSE);
    printf("D10 host: %u resolved rounds, terminal event=%u\n",
           (unsigned)ui.reveals,(unsigned)event);
}
int main(void)
{
    input_tests();
    layout_tests();
    start();
    combat_tests();
    puts("D10 C99 combat presentation tests PASS");
    return 0;
}
