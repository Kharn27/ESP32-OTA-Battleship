#include "warship_captain.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* The CPU is a consumer of the PUBLIC projection, not private battle state. */
typedef struct {uint32_t seed; unsigned calls; unsigned fail;} TestRandom;
static int sample(void *ctx, uint32_t *value)
{
    TestRandom *rng = ctx;
    ++rng->calls;
    if (rng->fail) return -1;
    uint32_t s = rng->seed;
    s ^= s << 13;
    s ^= s >> 17;
    s ^= s << 5;
    rng->seed = s;
    *value = s;
    return 0;
}
static int zero(void *ctx, uint32_t *value)
{
    (void)ctx;
    *value = 0u;
    return 0;
}
static int maximum(void *ctx, uint32_t *value)
{
    (void)ctx;
    *value = UINT32_MAX;
    return 0;
}
static void add(WarshipBattlePublic *view, unsigned x, unsigned y,
                uint8_t impact, uint8_t id)
{
    assert(view->outgoing_count < WARSHIP_BATTLE_CELLS);
    unsigned i = view->outgoing_count;
    WarshipBattleShot s = {(uint8_t)x,(uint8_t)y,impact,id};
    view->outgoing[i] = s;
    /* Independent but valid incoming history for tests; CPU may not use it. */
    view->incoming[i] = s;
    ++view->outgoing_count;
    ++view->incoming_count;
}
static void expect(const WarshipBattlePublic *v, unsigned x, unsigned y)
{
    WarshipCaptainTarget out = {0xabu,0xcdu};
    assert(warship_captain_choose(v, zero, NULL, &out) == WARSHIP_CAPTAIN_OK);
    assert(out.x == x && out.y == y);
}
static int adjacent(unsigned x, unsigned y, unsigned a, unsigned b)
{
    return (x == a && ((y + 1u == b) || (b + 1u == y))) ||
           (y == b && ((x + 1u == a) || (a + 1u == x)));
}
static void test_first_hit(void)
{
    for (unsigned mask = 0u; mask < 16u; ++mask) {
        WarshipBattlePublic v = {0};
        add(&v, 4u,4u,WARSHIP_BATTLE_HIT,WARSHIP_BATTLE_NO_SHIP);
        const unsigned nx[4] = {3u,5u,4u,4u};
        const unsigned ny[4] = {4u,4u,3u,5u};
        for (unsigned i = 0u; i < 4u; ++i)
            if (mask & (1u << i))
                add(&v,nx[i],ny[i],WARSHIP_BATTLE_MISS,WARSHIP_BATTLE_NO_SHIP);
        WarshipCaptainTarget out={0};
        assert(warship_captain_choose(&v,zero,NULL,&out)==WARSHIP_CAPTAIN_OK);
        if (mask != 15u) {
            assert(adjacent(out.x,out.y,4u,4u));
            for (unsigned i = 0u; i < 4u; ++i)
                if (mask & (1u << i)) assert(out.x!=nx[i] || out.y!=ny[i]);
        }
    }
    /* Corners and edges retain in-bounds orthogonal pursuit. */
    const unsigned sx[4]={0u,9u,0u,9u};
    const unsigned sy[4]={0u,0u,9u,9u};
    for(unsigned i=0u;i<4u;++i){
        WarshipBattlePublic v={0};
        add(&v,sx[i],sy[i],WARSHIP_BATTLE_HIT,WARSHIP_BATTLE_NO_SHIP);
        WarshipCaptainTarget out={0};
        assert(warship_captain_choose(&v,maximum,NULL,&out)==WARSHIP_CAPTAIN_OK);
        assert(out.x<10u && out.y<10u && adjacent(out.x,out.y,sx[i],sy[i]));
    }
    /* Neighbour MISS does not abandon E5 as an active clue. */
    WarshipBattlePublic v={0};
    add(&v,4,4,WARSHIP_BATTLE_HIT,WARSHIP_BATTLE_NO_SHIP);
    add(&v,3,4,WARSHIP_BATTLE_MISS,WARSHIP_BATTLE_NO_SHIP);
    WarshipCaptainTarget o={0};
    assert(warship_captain_choose(&v,maximum,NULL,&o)==WARSHIP_CAPTAIN_OK);
    assert(adjacent(o.x,o.y,4,4) && !(o.x==3u && o.y==4u));
}
static void test_aligned(void)
{
    WarshipBattlePublic v={0};
    add(&v,4,4,WARSHIP_BATTLE_HIT,WARSHIP_BATTLE_NO_SHIP);
    add(&v,5,4,WARSHIP_BATTLE_HIT,WARSHIP_BATTLE_NO_SHIP);
    WarshipCaptainTarget o={0};
    assert(warship_captain_choose(&v,maximum,NULL,&o)==WARSHIP_CAPTAIN_OK);
    assert((o.x==3u || o.x==6u) && o.y==4u);
    add(&v,3,4,WARSHIP_BATTLE_MISS,WARSHIP_BATTLE_NO_SHIP);
    expect(&v,6u,4u);

    WarshipBattlePublic w={0};
    add(&w,4,4,WARSHIP_BATTLE_HIT,WARSHIP_BATTLE_NO_SHIP);
    add(&w,5,4,WARSHIP_BATTLE_HIT,WARSHIP_BATTLE_NO_SHIP);
    add(&w,6,4,WARSHIP_BATTLE_MISS,WARSHIP_BATTLE_NO_SHIP);
    expect(&w,3u,4u);
    add(&w,3,4,WARSHIP_BATTLE_MISS,WARSHIP_BATTLE_NO_SHIP);
    assert(warship_captain_choose(&w,zero,NULL,&o)==WARSHIP_CAPTAIN_OK);
    assert(o.x<10u && o.y<10u && !(o.x==3u && o.y==4u) &&
           !(o.x==6u && o.y==4u));

    WarshipBattlePublic vertical={0};
    add(&vertical,4,4,WARSHIP_BATTLE_HIT,WARSHIP_BATTLE_NO_SHIP);
    add(&vertical,4,5,WARSHIP_BATTLE_HIT,WARSHIP_BATTLE_NO_SHIP);
    assert(warship_captain_choose(&vertical,zero,NULL,&o)==WARSHIP_CAPTAIN_OK);
    assert(o.x==4u && (o.y==3u || o.y==6u));
    add(&vertical,4,3,WARSHIP_BATTLE_MISS,WARSHIP_BATTLE_NO_SHIP);
    expect(&vertical,4u,6u);
}
static void test_search(void)
{
    unsigned distinct_first = 0u;
    unsigned first = 100u;
    for (uint32_t seed=1u;seed<=12u;++seed) {
        WarshipBattlePublic v={0};
        TestRandom rng={seed,0u,0u};
        uint8_t used[100]={0};
        unsigned quadrants[4]={0};
        for (unsigned step=0u;step<70u;++step) {
            WarshipCaptainTarget out={0};
            assert(warship_captain_choose(&v,sample,&rng,&out)==WARSHIP_CAPTAIN_OK);
            const unsigned idx=out.y*10u+out.x;
            assert(out.x<10u && out.y<10u && used[idx]==0);
            used[idx]=1;
            if (step==0u) {
                if(first!=idx){++distinct_first;first=idx;}
            }
            ++quadrants[(out.x>=5u?1u:0u)+(out.y>=5u?2u:0u)];
            add(&v,out.x,out.y,WARSHIP_BATTLE_MISS,WARSHIP_BATTLE_NO_SHIP);
        }
        unsigned smallest=70u,largest=0u;
        for(unsigned q=0;q<4u;++q){
            if(quadrants[q]<smallest)smallest=quadrants[q];
            if(quadrants[q]>largest)largest=quadrants[q];
        }
        assert(largest-smallest<=2u);
        assert(rng.calls==70u);
    }
    assert(distinct_first>=2u);
}
static void test_full_and_sparse(void)
{
    WarshipBattlePublic v={0};
    for(unsigned i=0u;i<100u;++i){
        if(i==87u)continue;
        add(&v,i%10u,i/10u,WARSHIP_BATTLE_MISS,WARSHIP_BATTLE_NO_SHIP);
    }
    expect(&v,7u,8u);
    add(&v,7u,8u,WARSHIP_BATTLE_MISS,WARSHIP_BATTLE_NO_SHIP);
    WarshipCaptainTarget out={71u,72u};
    TestRandom rng={42u,0u,0u};
    assert(warship_captain_choose(&v,sample,&rng,&out)==WARSHIP_CAPTAIN_NO_TARGET);
    assert(out.x==71u && out.y==72u && rng.calls==0u);
    v.verdict=WARSHIP_BATTLE_DRAW;
    assert(warship_captain_choose(&v,zero,NULL,&out)==WARSHIP_CAPTAIN_NO_TARGET);
}
static void check_error(WarshipBattlePublic *v, WarshipCaptainStatus expected,
                        WarshipCaptainRandom r, TestRandom *rng)
{
    WarshipCaptainTarget out={71u,72u};
    WarshipBattlePublic before=*v;
    assert(warship_captain_choose(v,r,rng,&out)==expected);
    assert(out.x==71u && out.y==72u);
    assert(memcmp(&before,v,sizeof before)==0);
}
static void test_invalid(void)
{
    WarshipBattlePublic v={0};
    WarshipCaptainTarget out={77u,88u};
    assert(warship_captain_choose(NULL,zero,NULL,&out)==WARSHIP_CAPTAIN_BAD_ARGUMENT);
    assert(warship_captain_choose(&v,NULL,NULL,&out)==WARSHIP_CAPTAIN_BAD_ARGUMENT);
    assert(warship_captain_choose(&v,zero,NULL,NULL)==WARSHIP_CAPTAIN_BAD_ARGUMENT);
    assert(out.x==77u && out.y==88u);
    TestRandom rng={1u,0u,1u};
    check_error(&v,WARSHIP_CAPTAIN_RANDOM_FAILURE,sample,&rng);
    assert(rng.calls==1u);
    v.outgoing_count=101u;
    check_error(&v,WARSHIP_CAPTAIN_BAD_OBSERVATION,zero,NULL);
    v.outgoing_count=0u;v.incoming_count=101u;
    check_error(&v,WARSHIP_CAPTAIN_BAD_OBSERVATION,zero,NULL);
    v.incoming_count=0u;v.verdict=4u;
    check_error(&v,WARSHIP_CAPTAIN_BAD_OBSERVATION,zero,NULL);
    v.verdict=WARSHIP_BATTLE_CONTINUE;
    add(&v,4,4,WARSHIP_BATTLE_HIT,WARSHIP_BATTLE_NO_SHIP);
    ++v.incoming_count;
    check_error(&v,WARSHIP_CAPTAIN_BAD_OBSERVATION,zero,NULL);
    --v.incoming_count;
    v.outgoing[0].x=10u;
    check_error(&v,WARSHIP_CAPTAIN_BAD_OBSERVATION,zero,NULL);
    v.outgoing[0].x=4u;v.incoming[0].y=10u;
    check_error(&v,WARSHIP_CAPTAIN_BAD_OBSERVATION,zero,NULL);
    v.incoming[0].y=4u;v.outgoing[0].impact=3u;
    check_error(&v,WARSHIP_CAPTAIN_BAD_OBSERVATION,zero,NULL);
    v.outgoing[0].impact=WARSHIP_BATTLE_HIT;v.outgoing[0].ship_id=0;
    check_error(&v,WARSHIP_CAPTAIN_BAD_OBSERVATION,zero,NULL);
    v.outgoing[0].ship_id=WARSHIP_BATTLE_NO_SHIP;
    add(&v,4,4,WARSHIP_BATTLE_MISS,WARSHIP_BATTLE_NO_SHIP);
    check_error(&v,WARSHIP_CAPTAIN_BAD_OBSERVATION,zero,NULL);
    v.outgoing[1].x=5u;
    v.outgoing[1].impact=WARSHIP_BATTLE_SUNK;
    v.outgoing[1].ship_id=6u;
    check_error(&v,WARSHIP_CAPTAIN_BAD_OBSERVATION,zero,NULL);
    v.outgoing[1].ship_id=WARSHIP_SHIP_PATROL;v.incoming[1].x=5u;
    assert(warship_captain_choose(&v,zero,NULL,&out)==WARSHIP_CAPTAIN_OK);
    add(&v,7,7,WARSHIP_BATTLE_SUNK,WARSHIP_SHIP_PATROL);
    check_error(&v,WARSHIP_CAPTAIN_BAD_OBSERVATION,zero,NULL);
    v.outgoing[2].ship_id=WARSHIP_SHIP_CARRIER;
    assert(warship_captain_choose(&v,maximum,NULL,&out)==WARSHIP_CAPTAIN_OK);
    v.incoming[1].ship_id=WARSHIP_SHIP_PATROL;
    check_error(&v,WARSHIP_CAPTAIN_BAD_OBSERVATION,zero,NULL);
}
static void test_incoming_irrelevant(void)
{
    WarshipBattlePublic v={0},w={0};
    add(&v,4,4,WARSHIP_BATTLE_HIT,WARSHIP_BATTLE_NO_SHIP);
    w=v;
    w.incoming[0].x=1;w.incoming[0].y=7;
    WarshipCaptainTarget a={0},b={0};
    assert(warship_captain_choose(&v,zero,NULL,&a)==WARSHIP_CAPTAIN_OK);
    assert(warship_captain_choose(&w,zero,NULL,&b)==WARSHIP_CAPTAIN_OK);
    assert(memcmp(&a,&b,sizeof a)==0);
}
#ifndef D03_STANDALONE
static WarshipFleet fleet(unsigned offset)
{
    WarshipFleet f={0};
    const uint8_t sizes[5]={2u,3u,3u,4u,5u};
    for(unsigned i=0;i<5u;++i){
        f.ships[i].id=(uint8_t)i;
        f.ships[i].x=(uint8_t)offset;
        f.ships[i].y=(uint8_t)(i*2u);
        f.ships[i].length=sizes[i];
        f.ships[i].orientation=WARSHIP_ORIENT_HORIZONTAL;
    }
    return f;
}
static void test_secret_invariance_real_d02(void)
{
    /* Distinct hidden HUMAN fleets, identical public outcomes at (9,9). */
    const WarshipFleet human_a=fleet(0u),human_b=fleet(3u),captain=fleet(0u);
    WarshipBattle b1,b2;
    WarshipBattlePublic p1={0},p2={0};
    WarshipCaptainTarget a={0},b={0};
    assert(warship_battle_init(&b1,&human_a,&captain)==WARSHIP_BATTLE_OK);
    assert(warship_battle_init(&b2,&human_b,&captain)==WARSHIP_BATTLE_OK);
    assert(warship_battle_observe(&b1,WARSHIP_BATTLE_CAPTAIN,&p1)==WARSHIP_BATTLE_OK);
    assert(warship_battle_observe(&b2,WARSHIP_BATTLE_CAPTAIN,&p2)==WARSHIP_BATTLE_OK);
    assert(memcmp(&p1,&p2,sizeof p1)==0);
    TestRandom r1={27u,0u,0u},r2=r1;
    assert(warship_captain_choose(&p1,sample,&r1,&a)==WARSHIP_CAPTAIN_OK);
    assert(warship_captain_choose(&p2,sample,&r2,&b)==WARSHIP_CAPTAIN_OK);
    assert(memcmp(&a,&b,sizeof a)==0);
    for (unsigned k=0;k<2u;++k){
        WarshipBattle *battle=k==0u?&b1:&b2;
        assert(warship_battle_select(battle,WARSHIP_BATTLE_HUMAN,9,9)==WARSHIP_BATTLE_OK);
        assert(warship_battle_select(battle,WARSHIP_BATTLE_CAPTAIN,9,9)==WARSHIP_BATTLE_OK);
        assert(warship_battle_fire(battle,WARSHIP_BATTLE_CAPTAIN)==WARSHIP_BATTLE_OK);
        assert(warship_battle_fire(battle,WARSHIP_BATTLE_HUMAN)==WARSHIP_BATTLE_OK);
        WarshipBattlePair pair;
        assert(warship_battle_resolve(battle,&pair)==WARSHIP_BATTLE_OK);
        assert(pair.captain.impact==WARSHIP_BATTLE_MISS);
    }
    assert(warship_battle_observe(&b1,WARSHIP_BATTLE_CAPTAIN,&p1)==WARSHIP_BATTLE_OK);
    assert(warship_battle_observe(&b2,WARSHIP_BATTLE_CAPTAIN,&p2)==WARSHIP_BATTLE_OK);
    assert(memcmp(&p1,&p2,sizeof p1)==0);
    r1.seed=1023u;r2=r1;
    assert(warship_captain_choose(&p1,sample,&r1,&a)==WARSHIP_CAPTAIN_OK);
    assert(warship_captain_choose(&p2,sample,&r2,&b)==WARSHIP_CAPTAIN_OK);
    assert(memcmp(&a,&b,sizeof a)==0);
}
#endif
int main(void)
{
    test_first_hit();
    test_aligned();
    test_search();
    test_full_and_sparse();
    test_invalid();
    test_incoming_irrelevant();
#ifndef D03_STANDALONE
    test_secret_invariance_real_d02();
#endif
    printf("D03 PASS sizeof(target)=%zu sizeof(public)=%zu\n",
           sizeof(WarshipCaptainTarget),sizeof(WarshipBattlePublic));
    return 0;
}
