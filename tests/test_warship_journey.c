#include "warship_journey.h"
#include "warship_deploy_ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if(!(x)) { fprintf(stderr,"D12 FAIL %s:%d %s\\n",__FILE__,__LINE__,#x); exit(EXIT_FAILURE); } } while(0)
#define EQ(a,b) CHECK((a)==(b))
#define OK(a) EQ((a),WARSHIP_LIFECYCLE_OK)
#define UI_OK(a) EQ((a),WARSHIP_COMBAT_OK)
#define NAV_OK(a) EQ((a),WARSHIP_JOURNEY_DONE)

typedef struct { uint32_t values[5];unsigned calls;int fail;} FleetRng;
static int fleet_rng(void *ctx,uint32_t *out)
{
    FleetRng *r=(FleetRng *)ctx;
    if(r->fail || !out) return -1;
    *out=r->values[r->calls++%5u];
    return 0;
}
static FleetRng human_rng(void)
{
    FleetRng v={{0u,20u,40u,60u,80u},0u,0};return v;
}
static FleetRng captain_rng(void)
{
    FleetRng v={{10u,30u,50u,70u,90u},0u,0};return v;
}
static FleetRng other_rng(void)
{
    FleetRng v={{12u,32u,52u,72u,92u},0u,0};return v;
}
static uint32_t random_state;
static int shoot_rng(void *ctx,uint32_t *out)
{
    (void)ctx;
    random_state^=random_state<<13;
    random_state^=random_state>>17;
    random_state^=random_state<<5;
    *out=random_state;
    return 0;
}
typedef struct {
    uint8_t blob[WARSHIP_PALMARES_WIRE_SIZE];
    size_t length;
    uint8_t exists;
    unsigned loads,stores;
    uint8_t fail_before,fail_after;
} Score;
typedef struct {
    uint64_t saved;
    uint8_t exists;
    unsigned loads,stores;
    uint8_t fail_before,fail_after;
} Sequence;
static int score_load(void *ctx,uint8_t *out,size_t cap,size_t *len)
{
    Score *s=(Score*)ctx;++s->loads;
    if(!s->exists)return 1;
    *len=s->length;
    if(s->length<=cap) memcpy(out,s->blob,s->length);
    return 0;
}
static int score_store(void *ctx,const uint8_t *data,size_t len)
{
    Score *s=(Score*)ctx;++s->stores;
    if(s->fail_before)return -1;
    CHECK(len==WARSHIP_PALMARES_WIRE_SIZE);
    memcpy(s->blob,data,len);
    s->length=len;s->exists=1;
    return s->fail_after?-1:0;
}
static int seq_load(void *ctx,uint64_t *id)
{
    Sequence *s=(Sequence*)ctx;++s->loads;
    if(!s->exists)return 1;
    *id=s->saved;return 0;
}
static int seq_save(void *ctx,uint64_t id)
{
    Sequence *s=(Sequence*)ctx;++s->stores;
    if(s->fail_before)return -1;
    s->saved=id;s->exists=1;
    return s->fail_after?-1:0;
}
typedef struct {
    Score score;Sequence seq;WarshipEventBridge ledger;
    unsigned delivered;
    WarshipLifecycleEvent last_delivered;
} Storage;
static void open_ledger(Storage *s)
{
    const WarshipPalmaresBackend sb={score_load,score_store};
    const WarshipEventIdBackend ib={seq_load,seq_save};
    EQ(warship_event_bridge_open(&s->ledger,sb,&s->score,ib,&s->seq),
       WARSHIP_EVENT_APPLIED);
}
static WarshipEventStatus deliver(void *ctx,WarshipLifecycleEvent e)
{
    Storage *s=(Storage*)ctx;
    ++s->delivered;s->last_delivered=e;
    return warship_event_bridge_receive(&s->ledger,e);
}
static void scores(Storage *s,uint32_t games,uint32_t wins,
                   uint32_t losses,uint32_t draws,uint32_t forfeits)
{
    WarshipPalmaresView v;
    EQ(warship_event_bridge_read(&s->ledger,&v),WARSHIP_EVENT_APPLIED);
    EQ(v.games,games);EQ(v.victories,wins);EQ(v.defeats,losses);
    EQ(v.draws,draws);EQ(v.voluntary_forfeits,forfeits);
    EQ(v.games,(uint64_t)v.victories+v.defeats+v.draws);
}
static WarshipLifecycle game;
static WarshipCombatUi combat;
static WarshipCombatInput touch;
static WarshipLifecyclePublic pub;
static WarshipCombatFrame frame;
static Storage store;
static void start(uint64_t ms)
{
    memset(&game,0,sizeof game);
    OK(warship_lifecycle_init(&game));
    FleetRng h=human_rng(),c=captain_rng();
    OK(warship_lifecycle_start(&game,fleet_rng,&h,fleet_rng,&c));
    EQ(h.calls,5u);EQ(c.calls,5u);
    EQ(game.phase,WARSHIP_LIFECYCLE_DEPLOY);
    random_state=UINT32_C(0x2468ace1);
    warship_combat_init(&combat,&touch,ms);
}
static void ready(void)
{
    OK(warship_lifecycle_ready(&game));
    EQ(game.phase,WARSHIP_LIFECYCLE_COMBAT);
}
static void observe(void)
{
    OK(warship_lifecycle_observe(&game,&pub));
}
static WarshipCombatHit h(uint8_t type,uint8_t x,uint8_t y)
{
    WarshipCombatHit v={type,x,y};return v;
}
static WarshipJourneyStatus navigate(uint8_t action,FleetRng *cpu,
                                     WarshipEventStatus *persisted)
{
    return warship_journey_apply(&game,&combat,h(action,0u,0u),
        cpu==NULL?NULL:fleet_rng,cpu,deliver,&store,persisted);
}
static void boot_storage(void)
{
    memset(&store,0,sizeof store);
    open_ledger(&store);
}
static void sample(unsigned down,unsigned valid,unsigned x,unsigned y,
                   uint64_t ms,WarshipCombatHit *out)
{
    UI_OK(warship_journey_input_step(&touch,&combat,(uint8_t)down,
        (uint8_t)valid,(uint16_t)x,(uint16_t)y,ms,out));
}
/* Finite tap. A long hold, release bounce and cross-phase contact
 * produce no second action. All times are monotonic. */
static WarshipCombatHit tap(uint64_t at,unsigned x,unsigned y)
{
    WarshipCombatHit v;
    sample(1u,1u,x,y,at,&v);EQ(v.command,WARSHIP_COMBAT_NO_HIT);
    sample(1u,1u,x,y,at+40u,&v);
    const WarshipCombatHit actual=v;
    sample(1u,1u,230u,40u,at+95u,&v);
    EQ(v.command,WARSHIP_COMBAT_NO_HIT);
    sample(0u,1u,0u,0u,at+105u,&v);
    sample(0u,1u,0u,0u,at+165u,&v);
    return actual;
}
static void test_pre_ready_and_forfeit(void)
{
    boot_storage();
    start(0u);
    FleetRng h_rng=human_rng();
    OK(warship_lifecycle_auto(&game,fleet_rng,&h_rng));
    EQ(warship_deploy_apply(&game,WARSHIP_DEPLOY_RETURN,NULL,NULL,NULL,NULL),
       WARSHIP_DEPLOY_OK);
    EQ(game.phase,WARSHIP_LIFECYCLE_HOME);
    EQ(store.score.stores,0u);EQ(store.seq.stores,0u);
    start(10u);ready();
    observe();
    const WarshipCombatHit pick=h(WARSHIP_COMBAT_CELL,3u,6u);
    UI_OK(warship_combat_apply(&combat,&game,&pub,pick,100u,shoot_rng,NULL));
    observe();
    CHECK(pub.battle.has_selection && pub.battle.target_x==3u &&
          pub.battle.target_y==6u);
    WarshipSoloSession prior=game.solo;

    /* COMBAT defense must still offer QUITTER. */
    WarshipEventStatus persisted=WARSHIP_EVENT_CONFLICT;
    EQ(navigate(WARSHIP_COMBAT_SWITCH,NULL,&persisted),
       WARSHIP_JOURNEY_NOT_HANDLED);
    UI_OK(warship_combat_apply(&combat,&game,&pub,
          h(WARSHIP_COMBAT_SWITCH,0u,0u),120u,shoot_rng,NULL));
    EQ(combat.view,WARSHIP_COMBAT_DEFENSE);
    /* READY was held when touch controller initialized. */
    WarshipCombatHit t;
    sample(0u,1u,0u,0u,150u,&t);
    sample(0u,1u,0u,0u,210u,&t);
    t=tap(240u,230u,150u);
    EQ(t.command,WARSHIP_COMBAT_CANCEL);
    NAV_OK(warship_journey_apply(&game,&combat,t,NULL,NULL,
        deliver,&store,&persisted));
    EQ(game.phase,WARSHIP_LIFECYCLE_CONFIRM_ABANDON);
    EQ(persisted,WARSHIP_EVENT_NO_CHANGE);
    CHECK(memcmp(&prior,&game.solo,sizeof prior)==0);
    EQ(store.delivered,0u);
    observe();
    UI_OK(warship_combat_make_frame(&pub,&combat,0u,&frame));
    EQ(frame.phase,WARSHIP_LIFECYCLE_CONFIRM_ABANDON);
    EQ(frame.view,WARSHIP_COMBAT_DEFENSE);
    unsigned public_occupied=0u;
    for(unsigned i=0u;i<100u;++i)
        public_occupied += frame.ships[i]!=0u;
    EQ(public_occupied,17u); /* DEFENSE shows HUMAN fleet only */
    CHECK(warship_combat_pixel(&frame,240u,50u)!=0u);
    EQ(navigate(WARSHIP_COMBAT_SWITCH,NULL,&persisted),WARSHIP_JOURNEY_IGNORED);
    EQ(navigate(WARSHIP_COMBAT_CELL,NULL,&persisted),WARSHIP_JOURNEY_IGNORED);

    /* CONTINUER preserves the complete private D06 session and selection.
     * A double tap while still held cannot become ABANDONNER. */
    /* The helper has already simulated full release; real runtime
     * discards the gesture at action time, not after release. */
    t=tap(460u,230u,50u);
    EQ(t.command,WARSHIP_COMBAT_FIRE);
    NAV_OK(warship_journey_apply(&game,&combat,t,NULL,NULL,
        deliver,&store,&persisted));
    EQ(game.phase,WARSHIP_LIFECYCLE_COMBAT);
    CHECK(memcmp(&prior,&game.solo,sizeof prior)==0);
    observe();
    CHECK(pub.battle.has_selection && pub.battle.target_x==3u &&
          pub.battle.target_y==6u);
    EQ(store.delivered,0u);

    /* QUITTER again -> user confirms ABANDONNER exactly once. */
    t=tap(680u,230u,150u);
    EQ(t.command,WARSHIP_COMBAT_CANCEL);
    NAV_OK(warship_journey_apply(&game,&combat,t,NULL,NULL,
        deliver,&store,&persisted));
    EQ(game.phase,WARSHIP_LIFECYCLE_CONFIRM_ABANDON);
    const WarshipLifecycle saved=game;
    const WarshipEventStatus old=persisted;
    CHECK(warship_journey_apply(&game,&combat,h(WARSHIP_COMBAT_CANCEL,0u,0u),
          NULL,NULL,NULL,NULL,&persisted)==WARSHIP_JOURNEY_BAD_ARGUMENT);
    CHECK(memcmp(&saved,&game,sizeof game)==0 && persisted==old);
    t=tap(900u,230u,150u);
    EQ(t.command,WARSHIP_COMBAT_CANCEL);
    NAV_OK(warship_journey_apply(&game,&combat,t,NULL,NULL,
        deliver,&store,&persisted));
    EQ(persisted,WARSHIP_EVENT_APPLIED);
    EQ(game.phase,WARSHIP_LIFECYCLE_HOME);
    EQ(store.delivered,1u);
    EQ(store.last_delivered,WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT);
    scores(&store,0u,0u,0u,0u,1u);
    EQ(store.score.stores,1u);
    EQ(navigate(WARSHIP_COMBAT_CANCEL,NULL,&persisted),WARSHIP_JOURNEY_IGNORED);
    EQ(store.delivered,1u);
    EQ(store.score.stores,1u);
}
static void test_fire_lock_no_deferred_quit(void)
{
    start(0u);ready();observe();
    UI_OK(warship_combat_apply(&combat,&game,&pub,
        h(WARSHIP_COMBAT_CELL,1u,1u),150u,shoot_rng,NULL));
    observe();
    UI_OK(warship_combat_apply(&combat,&game,&pub,
        h(WARSHIP_COMBAT_FIRE,0u,0u),200u,shoot_rng,NULL));
    CHECK(combat.pending);
    const WarshipLifecycle old=game;
    WarshipEventStatus persisted=WARSHIP_EVENT_NO_CHANGE;
    EQ(navigate(WARSHIP_COMBAT_CANCEL,NULL,&persisted),WARSHIP_JOURNEY_IGNORED);
    EQ(game.phase,WARSHIP_LIFECYCLE_COMBAT);
    CHECK(memcmp(&old,&game,sizeof old)==0);
    /* A physical QUITTER tap started DURING FEU is consumed forever,
     * even when reveal/ACK arrives before the finger releases. */
    WarshipCombatHit t;
    sample(0u,1u,0u,0u,230u,&t);
    sample(0u,1u,0u,0u,300u,&t);
    sample(1u,1u,230u,150u,330u,&t);
    sample(1u,1u,230u,150u,380u,&t);
    EQ(t.command,WARSHIP_COMBAT_NO_HIT);
    observe();
    UI_OK(warship_combat_make_frame(&pub,&combat,0u,&frame));
    CHECK(frame.busy);
    UI_OK(warship_combat_make_frame(&pub,&combat,1u,&frame));
    EQ(frame.ships[0],0u);
    warship_combat_input_discard(&touch);
    WarshipLifecycleEvent event=WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT;
    UI_OK(warship_combat_presented(&combat,&game,&event));
    EQ(event,WARSHIP_LIFECYCLE_EVENT_NONE);
    sample(1u,1u,230u,150u,430u,&t);
    EQ(t.command,WARSHIP_COMBAT_NO_HIT);
    sample(0u,1u,0u,0u,450u,&t);
    sample(0u,1u,0u,0u,520u,&t);
    EQ(game.phase,WARSHIP_LIFECYCLE_COMBAT);
    EQ(store.delivered,1u); /* previous test's forfeit only */
    /* Next independent tap is allowed after release, no queued QUIT. */
    t=tap(550u,230u,150u);
    EQ(t.command,WARSHIP_COMBAT_CANCEL);
}
static WarshipLifecycleEvent full_match(void)
{
    FleetRng unused=other_rng();
    WarshipLifecycleEvent e=WARSHIP_LIFECYCLE_EVENT_NONE;
    for(unsigned i=0u;i<100u && game.phase==WARSHIP_LIFECYCLE_COMBAT;++i) {
        observe();
        UI_OK(warship_combat_apply(&combat,&game,&pub,
            h(WARSHIP_COMBAT_CELL,(uint8_t)(i%10u),(uint8_t)(i/10u)),
            1000u+i*300u,shoot_rng,NULL));
        observe();
        UI_OK(warship_combat_apply(&combat,&game,&pub,
            h(WARSHIP_COMBAT_FIRE,0u,0u),
            1010u+i*300u,shoot_rng,NULL));
        CHECK(combat.pending);
        observe();
        UI_OK(warship_combat_make_frame(&pub,&combat,0u,&frame));
        CHECK(frame.busy);
        UI_OK(warship_combat_make_frame(&pub,&combat,1u,&frame));
        for(unsigned n=0u;n<100u;++n)
            CHECK(frame.ships[n]==0u); /* no CPU fleet on ATTAQUE */
        CHECK(warship_combat_reveal_due(&combat,1190u+i*300u));
        UI_OK(warship_combat_presented(&combat,&game,&e));
        if(e!=WARSHIP_LIFECYCLE_EVENT_NONE) {
            EQ(deliver(&store,e),WARSHIP_EVENT_APPLIED);
            break;
        }
    }
    EQ(game.phase,WARSHIP_LIFECYCLE_RESULT);
    CHECK(e==WARSHIP_LIFECYCLE_EVENT_VICTORY ||
          e==WARSHIP_LIFECYCLE_EVENT_DEFEAT ||
          e==WARSHIP_LIFECYCLE_EVENT_DRAW);
    (void)unused;
    return e;
}
static void test_result_replay_return(void)
{
    boot_storage();
    start(0u);ready();
    const WarshipLifecycleEvent result=full_match();
    EQ(store.delivered,1u);
    scores(&store,1u,result==WARSHIP_LIFECYCLE_EVENT_VICTORY,
        result==WARSHIP_LIFECYCLE_EVENT_DEFEAT,
        result==WARSHIP_LIFECYCLE_EVENT_DRAW,0u);
    observe();
    UI_OK(warship_combat_make_frame(&pub,&combat,0u,&frame));
    EQ(frame.phase,WARSHIP_LIFECYCLE_RESULT);
    EQ(frame.terminal,1u);
    CHECK(warship_combat_pixel(&frame,230u,50u)!=0u);
    WarshipEventStatus persisted=WARSHIP_EVENT_NO_CHANGE;
    EQ(navigate(WARSHIP_COMBAT_CELL,NULL,&persisted),WARSHIP_JOURNEY_IGNORED);
    EQ(navigate(WARSHIP_COMBAT_SWITCH,NULL,&persisted),
       WARSHIP_JOURNEY_NOT_HANDLED);
    const WarshipLifecycle saved=game;
    const WarshipCombatUi saved_ui=combat;
    FleetRng bad=other_rng();bad.fail=1;
    EQ(navigate(WARSHIP_COMBAT_FIRE,&bad,&persisted),
       WARSHIP_JOURNEY_GAME_ERROR);
    CHECK(memcmp(&saved,&game,sizeof game)==0);
    CHECK(memcmp(&saved_ui,&combat,sizeof combat)==0);
    EQ(store.delivered,1u);

    const WarshipFleet previous_human=game.human;
    const WarshipFleet previous_cpu=game.captain;
    FleetRng fresh=other_rng();
    NAV_OK(navigate(WARSHIP_COMBAT_FIRE,&fresh,&persisted));
    EQ(game.phase,WARSHIP_LIFECYCLE_DEPLOY);
    CHECK(memcmp(&game.human,&previous_human,sizeof game.human)==0);
    CHECK(memcmp(&game.captain,&previous_cpu,sizeof game.captain)!=0);
    EQ(fresh.calls,5u);
    EQ(combat.pending,0u);EQ(combat.reveals,0u);
    EQ(store.delivered,1u);
    WarshipLifecyclePublic p;
    OK(warship_lifecycle_observe(&game,&p));
    EQ(p.has_battle,0u);
    /* REJOUER => DEPLOY, AUTO possible, PRET required again. */
    FleetRng regen=human_rng();
    OK(warship_lifecycle_auto(&game,fleet_rng,&regen));
    EQ(game.phase,WARSHIP_LIFECYCLE_DEPLOY);
    OK(warship_lifecycle_leave_deploy(&game));
    EQ(game.phase,WARSHIP_LIFECYCLE_HOME);
    EQ(store.delivered,1u);EQ(store.score.stores,1u);

    /* Start a second complete game and choose ACCUEIL after verdict. */
    start(50000u);ready();(void)full_match();
    EQ(store.delivered,2u);
    const uint32_t total=2u;
    scores(&store,total,
           (result==WARSHIP_LIFECYCLE_EVENT_VICTORY)+
             (store.last_delivered==WARSHIP_LIFECYCLE_EVENT_VICTORY),
           (result==WARSHIP_LIFECYCLE_EVENT_DEFEAT)+
             (store.last_delivered==WARSHIP_LIFECYCLE_EVENT_DEFEAT),
           (result==WARSHIP_LIFECYCLE_EVENT_DRAW)+
             (store.last_delivered==WARSHIP_LIFECYCLE_EVENT_DRAW),0u);
    NAV_OK(navigate(WARSHIP_COMBAT_CANCEL,NULL,&persisted));
    EQ(game.phase,WARSHIP_LIFECYCLE_HOME);
    EQ(store.delivered,2u);EQ(store.score.stores,2u);
}
static void test_uncertain_persistence(void)
{
    boot_storage();
    start(0u);ready();
    WarshipEventStatus st=WARSHIP_EVENT_NO_CHANGE;
    NAV_OK(navigate(WARSHIP_COMBAT_CANCEL,NULL,&st));
    store.score.fail_before=1u;
    EQ(navigate(WARSHIP_COMBAT_CANCEL,NULL,&st),
       WARSHIP_JOURNEY_SCORE_UNCONFIRMED);
    EQ(st,WARSHIP_EVENT_WRITE_UNCERTAIN);
    EQ(game.phase,WARSHIP_LIFECYCLE_HOME);
    EQ(store.delivered,1u);
    CHECK(warship_event_bridge_has_pending(&store.ledger));
    WarshipPalmaresView view;
    EQ(warship_event_bridge_read(&store.ledger,&view),
       WARSHIP_EVENT_RELOAD_REQUIRED);
    EQ(navigate(WARSHIP_COMBAT_CANCEL,NULL,&st),WARSHIP_JOURNEY_IGNORED);
    EQ(store.delivered,1u);
    store.score.fail_before=0u;
    EQ(warship_event_bridge_retry(&store.ledger),WARSHIP_EVENT_APPLIED);
    scores(&store,0u,0u,0u,0u,1u);
    EQ(store.score.stores,2u);
    /* Second attempt: score commits, response uncertain, D11 reload
     * confirms same ID/type ALREADY_APPLIED without second write. */
    start(1000u);ready();
    NAV_OK(navigate(WARSHIP_COMBAT_CANCEL,NULL,&st));
    store.score.fail_after=1u;
    EQ(navigate(WARSHIP_COMBAT_CANCEL,NULL,&st),
       WARSHIP_JOURNEY_SCORE_UNCONFIRMED);
    EQ(st,WARSHIP_EVENT_WRITE_UNCERTAIN);
    store.score.fail_after=0u;
    EQ(warship_event_bridge_retry(&store.ledger),
       WARSHIP_EVENT_ALREADY_APPLIED);
    scores(&store,0u,0u,0u,0u,2u);
    EQ(store.delivered,2u);
}
int main(void)
{
    test_pre_ready_and_forfeit();
    test_fire_lock_no_deferred_quit();
    test_result_replay_return();
    test_uncertain_persistence();
    puts("D12 journey D07+D10+D11 host PASS");
    return EXIT_SUCCESS;
}
