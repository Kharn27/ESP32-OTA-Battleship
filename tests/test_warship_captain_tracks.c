#include "warship_captain.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct { uint32_t value; unsigned calls; int fail; } Rng;
static int random_sample(void *ctx, uint32_t *out)
{
    Rng *r=(Rng *)ctx;
    ++r->calls;
    if(r->fail) return -1;
    *out=r->value;
    return 0;
}
static void add(WarshipBattlePublic *v, unsigned x, unsigned y,
                unsigned impact, unsigned ship)
{
    assert(v->outgoing_count<WARSHIP_BATTLE_CELLS);
    const unsigned n=v->outgoing_count++;
    ++v->incoming_count;
    const WarshipBattleShot s={(uint8_t)x,(uint8_t)y,(uint8_t)impact,(uint8_t)ship};
    v->outgoing[n]=s;
    v->incoming[n]=s;
}
static WarshipCaptainTarget pick(const WarshipBattlePublic *v, uint32_t seed)
{
    WarshipCaptainTarget t={255u,255u};
    Rng rng={seed,0u,0};
    assert(warship_captain_choose(v,random_sample,&rng,&t)==WARSHIP_CAPTAIN_OK);
    assert(rng.calls==1u && t.x<10u && t.y<10u);
    for(unsigned i=0u;i<v->outgoing_count;++i)
        assert(v->outgoing[i].x!=t.x || v->outgoing[i].y!=t.y);
    return t;
}
static int neighbor(WarshipCaptainTarget t,unsigned x,unsigned y)
{
    return (t.x==x && (t.y+1u==y || y+1u==t.y)) ||
           (t.y==y && (t.x+1u==x || x+1u==t.x));
}
static void test_contradictions(void)
{
    WarshipBattlePublic v={0};
    add(&v,3,3,WARSHIP_BATTLE_HIT,255); /* D4 */
    add(&v,4,3,WARSHIP_BATTLE_HIT,255); /* E4 */
    add(&v,2,3,WARSHIP_BATTLE_MISS,255); /* C4 */
    add(&v,5,3,WARSHIP_BATTLE_MISS,255); /* F4 */
    for(unsigned s=0u;s<8u;++s){
        const WarshipCaptainTarget t=pick(&v,s);
        assert((t.x==3u || t.x==4u) && (t.y==2u || t.y==4u));
    }
    add(&v,3,2,WARSHIP_BATTLE_MISS,255);
    WarshipCaptainTarget t=pick(&v,0u);
    assert((t.x==3u || t.x==4u) && (t.y==2u || t.y==4u));
    WarshipBattlePublic w={0};
    add(&w,6,3,WARSHIP_BATTLE_HIT,255);
    add(&w,6,4,WARSHIP_BATTLE_HIT,255);
    add(&w,6,2,WARSHIP_BATTLE_MISS,255);
    add(&w,6,5,WARSHIP_BATTLE_MISS,255);
    t=pick(&w,2u);
    assert((t.x==5u || t.x==7u) && (t.y==3u || t.y==4u));
    add(&w,5,3,WARSHIP_BATTLE_MISS,255);
    add(&w,7,3,WARSHIP_BATTLE_MISS,255);
    add(&w,5,4,WARSHIP_BATTLE_MISS,255);
    add(&w,7,4,WARSHIP_BATTLE_MISS,255);
    (void)pick(&w,0u); /* no remaining local extension, legal fallback */
}
static void test_two_tracks(void)
{
    for(unsigned reverse=0u;reverse<2u;++reverse){
        WarshipBattlePublic v={0};
        const unsigned first=reverse?7u:2u, second=reverse?2u:7u;
        add(&v,first,first,WARSHIP_BATTLE_HIT,255);
        add(&v,second,second,WARSHIP_BATTLE_HIT,255);
        assert(neighbor(pick(&v,0u),second,second));
        add(&v,second-1u,second,WARSHIP_BATTLE_MISS,255);
        assert(neighbor(pick(&v,1u),second,second));
        add(&v,second+1u,second,WARSHIP_BATTLE_MISS,255);
        add(&v,second,second-1u,WARSHIP_BATTLE_MISS,255);
        add(&v,second,second+1u,WARSHIP_BATTLE_MISS,255);
        assert(neighbor(pick(&v,2u),first,first));
    }
    WarshipBattlePublic v={0};
    add(&v,1,1,WARSHIP_BATTLE_HIT,255);
    add(&v,5,5,WARSHIP_BATTLE_HIT,255);
    add(&v,6,5,WARSHIP_BATTLE_HIT,255);
    WarshipCaptainTarget t=pick(&v,1u);
    assert(t.y==5u && (t.x==4u || t.x==7u));
    add(&v,4,5,WARSHIP_BATTLE_MISS,255);
    t=pick(&v,0u);
    assert(t.x==7u && t.y==5u); /* persistence after one blocked end */
}
static void test_sunk_without_wreck_map(void)
{
    WarshipBattlePublic v={0};
    add(&v,1,1,WARSHIP_BATTLE_HIT,255);
    add(&v,4,4,WARSHIP_BATTLE_HIT,255);
    add(&v,4,5,WARSHIP_BATTLE_SUNK,0);
    assert(neighbor(pick(&v,0u),1u,1u)); /* other public lead first */
    add(&v,0,1,WARSHIP_BATTLE_MISS,255);
    add(&v,2,1,WARSHIP_BATTLE_MISS,255);
    add(&v,1,0,WARSHIP_BATTLE_MISS,255);
    add(&v,1,2,WARSHIP_BATTLE_MISS,255);
    assert(neighbor(pick(&v,0u),4u,4u)); /* ambiguous lead recovered */
    WarshipBattlePublic a={0};
    add(&a,3,4,WARSHIP_BATTLE_HIT,255);
    add(&a,4,4,WARSHIP_BATTLE_HIT,255);
    add(&a,5,4,WARSHIP_BATTLE_HIT,255);
    add(&a,6,4,WARSHIP_BATTLE_SUNK,1); /* announced 3-cell type */
    WarshipCaptainTarget t=pick(&a,0u);
    assert(t.x==2u && t.y==4u); /* four hits, one remaining unknown */
    WarshipBattlePublic d={0};
    add(&d,4,4,WARSHIP_BATTLE_HIT,255);
    add(&d,5,5,WARSHIP_BATTLE_SUNK,1); /* diagonal contact possible */
    assert(neighbor(pick(&d,0u),4u,4u));
    WarshipBattlePublic b=a; /* different PUBLIC incoming, same decision */
    for(unsigned i=0u;i<b.incoming_count;++i){
        b.incoming[i].x=9u;
        b.incoming[i].y=(uint8_t)(9u-i);
    }
    const WarshipCaptainTarget u=pick(&a,101u),q=pick(&b,101u);
    assert(memcmp(&u,&q,sizeof u)==0);
}
static void test_bounds_and_errors(void)
{
    WarshipBattlePublic v={0};
    for(unsigned i=0u;i<100u;++i){
        if(i==42u)continue;
        add(&v,i%10u,i/10u,WARSHIP_BATTLE_MISS,255);
    }
    WarshipCaptainTarget t=pick(&v,UINT32_MAX);
    assert(t.x==2u && t.y==4u);
    add(&v,2,4,WARSHIP_BATTLE_MISS,255);
    Rng rng={0u,0u,0};
    WarshipCaptainTarget out={63u,64u};
    assert(warship_captain_choose(&v,random_sample,&rng,&out)==WARSHIP_CAPTAIN_NO_TARGET);
    assert(rng.calls==0u && out.x==63u && out.y==64u);
    v.outgoing_count=101u;
    assert(warship_captain_choose(&v,random_sample,&rng,&out)==WARSHIP_CAPTAIN_BAD_OBSERVATION);
    v.outgoing_count=100u;v.outgoing[0].x=10u;
    assert(warship_captain_choose(&v,random_sample,&rng,&out)==WARSHIP_CAPTAIN_BAD_OBSERVATION);
    v.outgoing[0].x=0u;v.outgoing[1]=v.outgoing[0];
    assert(warship_captain_choose(&v,random_sample,&rng,&out)==WARSHIP_CAPTAIN_BAD_OBSERVATION);
    WarshipBattlePublic empty={0};rng.fail=1;
    assert(warship_captain_choose(&empty,random_sample,&rng,&out)==WARSHIP_CAPTAIN_RANDOM_FAILURE);
    assert(out.x==63u && out.y==64u);
}
int main(void)
{
    test_contradictions();test_two_tracks();test_sunk_without_wreck_map();
    test_bounds_and_errors();
    printf("D04 PASS sizeof(target)=%zu sizeof(public)=%zu\n",
           sizeof(WarshipCaptainTarget),sizeof(WarshipBattlePublic));
    return 0;
}
