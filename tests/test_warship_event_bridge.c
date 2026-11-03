#include "warship_event_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"D11 FAIL %s:%d %s\n",__FILE__,__LINE__,#x); exit(EXIT_FAILURE); } } while (0)
#define EQ(a,b) CHECK((a)==(b))
#define LOK(a) EQ((a),WARSHIP_LIFECYCLE_OK)
#define EOK(a) EQ((a),WARSHIP_EVENT_APPLIED)

typedef struct {
    uint8_t bytes[WARSHIP_PALMARES_WIRE_SIZE];
    size_t length;
    unsigned loads, stores;
    int exists, read_fail, write_before_fail, write_after_fail;
} ScoreNvs;
typedef struct {
    uint64_t value;
    unsigned loads, stores;
    int exists, read_failure, write_before_fail, write_after_fail;
} IdNvs;
static int score_load(void *ctx, uint8_t *out,size_t capacity,size_t *actual)
{
    ScoreNvs *s=(ScoreNvs*)ctx;
    ++s->loads;
    if(s->read_fail) return -1;
    if(!s->exists) return 1;
    *actual=s->length;
    const size_t n=s->length<capacity?s->length:capacity;
    memcpy(out,s->bytes,n);
    return 0;
}
static int score_store(void *ctx,const uint8_t *data,size_t size)
{
    ScoreNvs *s=(ScoreNvs*)ctx;
    ++s->stores;
    if(s->write_before_fail)return -1;
    CHECK(size==WARSHIP_PALMARES_WIRE_SIZE);
    memcpy(s->bytes,data,size);
    s->length=size;
    s->exists=1;
    return s->write_after_fail?-1:0;
}
static int id_load(void *ctx,uint64_t *out)
{
    IdNvs *s=(IdNvs*)ctx;
    ++s->loads;
    if(s->read_failure)return s->read_failure;
    if(!s->exists)return 1;
    *out=s->value;
    return 0;
}
static int id_reserve(void *ctx,uint64_t id)
{
    IdNvs *s=(IdNvs*)ctx;
    ++s->stores;
    if(s->write_before_fail)return -1;
    CHECK(id!=0u);
    s->value=id;s->exists=1;
    return s->write_after_fail?-1:0;
}
static const WarshipPalmaresBackend score_backend={score_load,score_store};
static const WarshipEventIdBackend id_backend={id_load,id_reserve};
static WarshipEventBridge opened(ScoreNvs *s,IdNvs *i)
{
    WarshipEventBridge b;
    memset(&b,0x98,sizeof b);
    EOK(warship_event_bridge_open(&b,score_backend,s,id_backend,i));
    return b;
}
static WarshipPalmaresView view(const WarshipEventBridge *b)
{
    WarshipPalmaresView v;
    memset(&v,0xb5,sizeof v);
    EOK(warship_event_bridge_read(b,&v));
    EQ((uint64_t)v.victories+v.defeats+v.draws,v.games);
    return v;
}
static void check(const WarshipEventBridge *b,uint32_t games,uint32_t wins,
                  uint32_t loses,uint32_t draws,uint32_t forfeits)
{
    WarshipPalmaresView v=view(b);
    EQ(v.games,games); EQ(v.victories,wins);EQ(v.defeats,loses);
    EQ(v.draws,draws);EQ(v.voluntary_forfeits,forfeits);
}

/* Real D07 fixtures: the same deterministic CPU trace can end V/D/DRAW. */
typedef struct {uint32_t values[5];unsigned calls;} FleetRng;
static int fleet_rng(void *ctx,uint32_t *out)
{
    FleetRng *r=(FleetRng*)ctx;
    *out=r->values[r->calls++%5u];
    return 0;
}
static int cpu_rng(void *ctx,uint32_t *out)
{
    (void)ctx;*out=0u;return 0;
}
static WarshipLifecycle fresh_game(int ready)
{
    WarshipLifecycle g;
    LOK(warship_lifecycle_init(&g));
    FleetRng h={{0u,20u,40u,60u,80u},0u};
    FleetRng c={{10u,30u,50u,70u,90u},0u};
    LOK(warship_lifecycle_start(&g,fleet_rng,&h,fleet_rng,&c));
    if(ready) LOK(warship_lifecycle_ready(&g));
    return g;
}
static const uint8_t lengths[5]={2u,3u,3u,4u,5u};
static void ship_cell(unsigned ordinal,uint8_t *x,uint8_t *y)
{
    for(unsigned i=0u;i<5u;++i) {
        if(ordinal<lengths[i]){
            *x=(uint8_t)(5u+ordinal);*y=(uint8_t)i;return;
        }
        ordinal-=lengths[i];
    }
    CHECK(0);
}
static int occupied(unsigned x,unsigned y)
{
    return y<5u && x>=5u && x<5u+lengths[y];
}
static void water_cell(unsigned ordinal,uint8_t *x,uint8_t *y)
{
    for(unsigned p=0u;p<100u;++p) {
        if(occupied(p%10u,p/10u))continue;
        if(ordinal==0u){
            *x=(uint8_t)(p%10u);*y=(uint8_t)(p/10u);return;
        }
        --ordinal;
    }
    CHECK(0);
}
static unsigned finish(WarshipEventBridge *bridge,unsigned mode,
                       unsigned draw_at,WarshipBattleShot trace[100])
{
    WarshipLifecycle g=fresh_game(1);
    const WarshipLifecycleEvent expected=mode==0u?
        WARSHIP_LIFECYCLE_EVENT_VICTORY:mode==1u?
        WARSHIP_LIFECYCLE_EVENT_DEFEAT:WARSHIP_LIFECYCLE_EVENT_DRAW;
    for(unsigned n=0u;n<100u;++n) {
        uint8_t x=0u,y=0u;
        if(mode==0u || n<16u ||
           (mode==2u && n+1u==draw_at))
            ship_cell(n<16u?n:16u,&x,&y);
        else water_cell(n-16u,&x,&y);
        LOK(warship_lifecycle_select(&g,x,y));
        WarshipBattlePair pair;
        LOK(warship_lifecycle_fire(&g,cpu_rng,NULL,&pair));
        if(mode==1u)trace[n]=pair.captain;
        if(mode==2u) {
            EQ(trace[n].x,pair.captain.x);
            EQ(trace[n].y,pair.captain.y);
            EQ(trace[n].impact,pair.captain.impact);
        }
        /* Before visible presentation D07 MUST NOT deliver an event. */
        WarshipLifecyclePublic p;
        LOK(warship_lifecycle_observe(&g,&p));
        EQ(p.battle.history.outgoing_count,n);
        EQ(p.battle.history.incoming_count,n);
        EQ(warship_event_bridge_receive(bridge,
           WARSHIP_LIFECYCLE_EVENT_NONE),WARSHIP_EVENT_NO_CHANGE);
        WarshipLifecycleEvent event=WARSHIP_LIFECYCLE_EVENT_NONE;
        LOK(warship_lifecycle_ack_reveal(&g,&event));
        if(pair.verdict==WARSHIP_BATTLE_CONTINUE) {
            EQ(event,WARSHIP_LIFECYCLE_EVENT_NONE);
            EQ(warship_event_bridge_receive(bridge,event),
               WARSHIP_EVENT_NO_CHANGE);
            continue;
        }
        EQ(event,expected);
        EOK(warship_event_bridge_receive(bridge,event));
        EQ(warship_lifecycle_ack_reveal(&g,&event),
           WARSHIP_LIFECYCLE_BAD_PHASE);
        LOK(warship_lifecycle_leave_result(&g));
        return n+1u;
    }
    CHECK(0);
    return 0u;
}
static void test_real_d07_and_restart(void)
{
    ScoreNvs score={0};IdNvs ids={0};
    WarshipEventBridge b=opened(&score,&ids);
    WarshipLifecycle g=fresh_game(0);
    LOK(warship_lifecycle_leave_deploy(&g));
    EQ(warship_event_bridge_receive(&b,
       WARSHIP_LIFECYCLE_EVENT_NONE),WARSHIP_EVENT_NO_CHANGE);
    check(&b,0u,0u,0u,0u,0u);
    g=fresh_game(1);
    LOK(warship_lifecycle_request_abandon(&g));
    WarshipLifecycleEvent event=WARSHIP_LIFECYCLE_EVENT_VICTORY;
    LOK(warship_lifecycle_decide_abandon(&g,0u,&event));
    EQ(event,WARSHIP_LIFECYCLE_EVENT_NONE);
    EQ(warship_event_bridge_receive(&b,event),WARSHIP_EVENT_NO_CHANGE);
    LOK(warship_lifecycle_request_abandon(&g));
    LOK(warship_lifecycle_decide_abandon(&g,1u,&event));
    EQ(event,WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT);
    EOK(warship_event_bridge_receive(&b,event));
    check(&b,0u,0u,0u,0u,1u);
    EQ(score.stores,1u);
    EQ(ids.stores,1u);
    EQ(warship_lifecycle_decide_abandon(&g,1u,&event),
       WARSHIP_LIFECYCLE_BAD_PHASE);
    check(&b,0u,0u,0u,0u,1u);
    WarshipBattleShot trace[100]={{0}};
    EQ(finish(&b,0u,0u,trace),17u);
    check(&b,1u,1u,0u,0u,1u);
    const unsigned loss_at=finish(&b,1u,0u,trace);
    CHECK(loss_at>=17u&&loss_at<100u);
    check(&b,2u,1u,1u,0u,1u);
    EQ(finish(&b,2u,loss_at,trace),loss_at);
    check(&b,3u,1u,1u,1u,1u);
    EQ(score.stores,4u);EQ(ids.stores,4u);
    WarshipEventBridge restarted=opened(&score,&ids);
    check(&restarted,3u,1u,1u,1u,1u);
    uint64_t last=0u;WarshipLifecycleEvent type=WARSHIP_LIFECYCLE_EVENT_NONE;
    EOK(warship_event_bridge_last(&restarted,&last,&type));
    EQ(last,4u);EQ(type,WARSHIP_LIFECYCLE_EVENT_DRAW);
    EQ(warship_palmares_apply(&restarted.ledger,4u,type),
       WARSHIP_PALMARES_ALREADY_APPLIED);
    EQ(score.stores,4u);
    EOK(warship_event_bridge_receive(&restarted,
       WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT));
    EOK(warship_event_bridge_last(&restarted,&last,&type));
    EQ(last,5u);
    check(&restarted,3u,1u,1u,1u,2u);
}

static void test_retry_and_ambiguous_commits(void)
{
    ScoreNvs score={0};IdNvs ids={0};
    WarshipEventBridge b=opened(&score,&ids);
    ids.write_before_fail=1;
    EQ(warship_event_bridge_receive(&b,
       WARSHIP_LIFECYCLE_EVENT_VICTORY),WARSHIP_EVENT_WRITE_UNCERTAIN);
    CHECK(warship_event_bridge_has_pending(&b));
    EQ(score.stores,0u);
    EQ(warship_event_bridge_receive(&b,
       WARSHIP_LIFECYCLE_EVENT_DEFEAT),WARSHIP_EVENT_PENDING);
    EQ(warship_event_bridge_receive(&b,
       WARSHIP_LIFECYCLE_EVENT_VICTORY),WARSHIP_EVENT_PENDING);
    EQ(b.pending_id,1u);
    ids.write_before_fail=0;
    EOK(warship_event_bridge_retry(&b));
    EQ(ids.value,1u);EQ(score.stores,1u);
    /* New reservation itself committed, but firmware saw an error. */
    ids.write_after_fail=1;
    EQ(warship_event_bridge_receive(&b,
       WARSHIP_LIFECYCLE_EVENT_DEFEAT),WARSHIP_EVENT_WRITE_UNCERTAIN);
    EQ(ids.value,2u);EQ(score.stores,1u);
    ids.write_after_fail=0;
    EOK(warship_event_bridge_retry(&b));
    EQ(ids.stores,3u);EQ(score.stores,2u);
    check(&b,2u,1u,1u,0u,0u);
    score.write_after_fail=1;
    EQ(warship_event_bridge_receive(&b,
       WARSHIP_LIFECYCLE_EVENT_DRAW),WARSHIP_EVENT_WRITE_UNCERTAIN);
    EQ(b.pending_id,3u);
    uint64_t unchanged_id=UINT64_MAX;
    WarshipLifecycleEvent unchanged_event=WARSHIP_LIFECYCLE_EVENT_DEFEAT;
    EQ(warship_event_bridge_last(&b,&unchanged_id,&unchanged_event),
       WARSHIP_EVENT_RELOAD_REQUIRED);
    EQ(unchanged_id,UINT64_MAX);
    score.write_after_fail=0;
    EQ(warship_event_bridge_retry(&b),WARSHIP_EVENT_ALREADY_APPLIED);
    EQ(score.stores,3u);EQ(ids.stores,4u);
    check(&b,3u,1u,1u,1u,0u);
    CHECK(!warship_event_bridge_has_pending(&b));
    /* Write failed BEFORE persisting, then same ID must be retried. */
    score.write_before_fail=1;
    EQ(warship_event_bridge_receive(&b,
       WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT),
       WARSHIP_EVENT_WRITE_UNCERTAIN);
    EQ(b.pending_id,4u);
    score.write_before_fail=0;
    EOK(warship_event_bridge_retry(&b));
    EQ(ids.stores,5u);
    check(&b,3u,1u,1u,1u,1u);
}
static void test_fail_closed(void)
{
    ScoreNvs score={0};IdNvs ids={0};
    WarshipEventBridge b;
    memset(&b,0xa9,sizeof b);
    WarshipEventBridge old=b;
    EQ(warship_event_bridge_open(NULL,score_backend,&score,
       id_backend,&ids),WARSHIP_EVENT_BAD_ARGUMENT);
    CHECK(memcmp(&old,&b,sizeof b)==0);
    ids.read_failure=-3;
    EQ(warship_event_bridge_open(&b,score_backend,&score,
       id_backend,&ids),WARSHIP_EVENT_INCOMPATIBLE_VERSION);
    CHECK(memcmp(&old,&b,sizeof b)==0);
    ids.read_failure=-2;
    EQ(warship_event_bridge_open(&b,score_backend,&score,
       id_backend,&ids),WARSHIP_EVENT_CORRUPT);
    ids.read_failure=-1;
    EQ(warship_event_bridge_open(&b,score_backend,&score,
       id_backend,&ids),WARSHIP_EVENT_READ_ERROR);
    ids.read_failure=0;
    score.read_fail=1;
    EQ(warship_event_bridge_open(&b,score_backend,&score,
       id_backend,&ids),WARSHIP_EVENT_READ_ERROR);
    score.read_fail=0;
    score.exists=1;score.length=43u;
    EQ(warship_event_bridge_open(&b,score_backend,&score,
       id_backend,&ids),WARSHIP_EVENT_CORRUPT);
    score.exists=0;score.length=0u;
    ids.exists=1;ids.value=UINT64_MAX;
    EOK(warship_event_bridge_open(&b,score_backend,&score,id_backend,&ids));
    EQ(warship_event_bridge_receive(&b,
       WARSHIP_LIFECYCLE_EVENT_VICTORY),WARSHIP_EVENT_OVERFLOW);
    EQ(ids.stores,0u);EQ(score.stores,0u);
    EQ(warship_event_bridge_receive(&b,(WarshipLifecycleEvent)44),
       WARSHIP_EVENT_BAD_EVENT);
    EQ(warship_event_bridge_receive(&b,
       WARSHIP_LIFECYCLE_EVENT_NONE),WARSHIP_EVENT_NO_CHANGE);
}
static void test_restart_reserved_gap(void)
{
    ScoreNvs score={0};IdNvs ids={0};
    WarshipEventBridge b=opened(&score,&ids);
    EOK(warship_event_bridge_receive(&b,
       WARSHIP_LIFECYCLE_EVENT_VICTORY));
    /* Reservation for the next event commits, but score does not.
     * On reboot a distinct event MUST NOT reuse this reserved ID. */
    score.write_before_fail=1;
    EQ(warship_event_bridge_receive(&b,
       WARSHIP_LIFECYCLE_EVENT_DEFEAT),WARSHIP_EVENT_WRITE_UNCERTAIN);
    EQ(b.pending_id,2u);EQ(ids.value,2u);
    score.write_before_fail=0;
    WarshipEventBridge boot=opened(&score,&ids);
    EOK(warship_event_bridge_receive(&boot,
       WARSHIP_LIFECYCLE_EVENT_DRAW));
    EQ(ids.value,3u);
    check(&boot,2u,1u,0u,1u,0u);
    /* Existing D08 snapshots without reservation key migrate lazily
     * without any boot write and without recycling last_id. */
    ids.exists=0;
    WarshipEventBridge migrated=opened(&score,&ids);
    EQ(migrated.reserved_id,3u);
    EOK(warship_event_bridge_receive(&migrated,
       WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT));
    EQ(ids.value,4u);
    ids.value=2u;
    memset(&b,0xb4,sizeof b);
    EQ(warship_event_bridge_open(&b,score_backend,&score,
       id_backend,&ids),WARSHIP_EVENT_CORRUPT);
}
int main(void)
{
    test_real_d07_and_restart();
    test_retry_and_ambiguous_commits();
    test_fail_closed();
    test_restart_reserved_gap();
    printf("D11 C99 D07 events / durable ID / D08 scores PASS\n");
    return 0;
}
