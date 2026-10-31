#include "warship_battle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(c) do { if (!(c)) {fprintf(stderr,"FAIL %s:%d %s\n",__FILE__,__LINE__,#c);exit(EXIT_FAILURE);} } while (0)
#define EQ(a,b) CHECK((a)==(b))
#define OK(s) EQ((s),WARSHIP_BATTLE_OK)

static const WarshipFleet fleet0 = {{
 {WARSHIP_SHIP_PATROL,0u,0u,2u,WARSHIP_ORIENT_HORIZONTAL},
 {WARSHIP_SHIP_SUBMARINE,0u,1u,3u,WARSHIP_ORIENT_HORIZONTAL},
 {WARSHIP_SHIP_DESTROYER,0u,2u,3u,WARSHIP_ORIENT_HORIZONTAL},
 {WARSHIP_SHIP_CRUISER,0u,3u,4u,WARSHIP_ORIENT_HORIZONTAL},
 {WARSHIP_SHIP_CARRIER,0u,4u,5u,WARSHIP_ORIENT_HORIZONTAL}
}};
static const WarshipFleet fleet1 = {{
 {WARSHIP_SHIP_PATROL,5u,0u,2u,WARSHIP_ORIENT_HORIZONTAL},
 {WARSHIP_SHIP_SUBMARINE,5u,1u,3u,WARSHIP_ORIENT_HORIZONTAL},
 {WARSHIP_SHIP_DESTROYER,5u,2u,3u,WARSHIP_ORIENT_HORIZONTAL},
 {WARSHIP_SHIP_CRUISER,5u,3u,4u,WARSHIP_ORIENT_HORIZONTAL},
 {WARSHIP_SHIP_CARRIER,5u,4u,5u,WARSHIP_ORIENT_HORIZONTAL}
}};
static const uint8_t ship_lengths[5] = {2u,3u,3u,4u,5u};

static WarshipBattle new_battle(const WarshipFleet *human, const WarshipFleet *captain)
{
 WarshipBattle battle; memset(&battle,0x7e,sizeof battle);
 OK(warship_battle_init(&battle,human,captain));
 EQ(warship_battle_phase(&battle),WARSHIP_BATTLE_PREPARING);
 return battle;
}

static void check_unchanged(WarshipBattle *b, WarshipBattleStatus got, WarshipBattleStatus expected, const WarshipBattle *before)
{
 EQ(got,expected);CHECK(memcmp(b,before,sizeof *b)==0);
}

static void invalid_init(void)
{
 WarshipBattle b;memset(&b,0xa5,sizeof b);WarshipBattle saved=b;
 EQ(warship_battle_init(NULL,&fleet0,&fleet1),WARSHIP_BATTLE_BAD_ARGUMENT);
 EQ(warship_battle_init(&b,NULL,&fleet1),WARSHIP_BATTLE_BAD_ARGUMENT);
 CHECK(memcmp(&saved,&b,sizeof b)==0);
 EQ(warship_battle_init(&b,&fleet0,NULL),WARSHIP_BATTLE_BAD_ARGUMENT);
 CHECK(memcmp(&saved,&b,sizeof b)==0);
 WarshipFleet bad=fleet0;bad.ships[0].x=10u;
 EQ(warship_battle_init(&b,&bad,&fleet1),WARSHIP_BATTLE_BAD_FLEET);
 CHECK(memcmp(&saved,&b,sizeof b)==0);
 EQ(warship_battle_init(&b,&fleet0,&bad),WARSHIP_BATTLE_BAD_FLEET);
 CHECK(memcmp(&saved,&b,sizeof b)==0);
 bad=fleet1;bad.ships[4].id=WARSHIP_SHIP_PATROL;
 EQ(warship_battle_init(&b,&fleet0,&bad),WARSHIP_BATTLE_BAD_FLEET);
 CHECK(memcmp(&saved,&b,sizeof b)==0);
 OK(warship_battle_init(&b,&fleet0,&fleet1));
 WarshipFleet changed=fleet0;changed.ships[0].x=8u;
 /* Re-initializing from a mutable stack copy, then mutating it externally. */
 OK(warship_battle_init(&b,&changed,&fleet1));
 changed.ships[0].x=255u;
 WarshipBattle before=b;
 EQ(warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,0u,5u),WARSHIP_BATTLE_OK);
 CHECK(memcmp(&b.fleets[WARSHIP_BATTLE_HUMAN],&before.fleets[WARSHIP_BATTLE_HUMAN],sizeof(WarshipFleet))==0);
}

static void selection_and_secrecy(void)
{
 WarshipBattle b=new_battle(&fleet0,&fleet1);
 WarshipBattle save=b;
 check_unchanged(&b,warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,10u,0u),WARSHIP_BATTLE_OUT_OF_BOUNDS,&save);
 check_unchanged(&b,warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,0u,255u),WARSHIP_BATTLE_OUT_OF_BOUNDS,&save);
 check_unchanged(&b,warship_battle_select(&b,(WarshipBattleSide)255,0u,0u),WARSHIP_BATTLE_BAD_SIDE,&save);
 check_unchanged(&b,warship_battle_fire(&b,WARSHIP_BATTLE_HUMAN),WARSHIP_BATTLE_NO_SELECTION,&save);
 check_unchanged(&b,warship_battle_cancel(&b,WARSHIP_BATTLE_HUMAN),WARSHIP_BATTLE_NO_SELECTION,&save);
 OK(warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,9u,9u));
 save=b;
 check_unchanged(&b,warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,9u,9u),WARSHIP_BATTLE_DUPLICATE_SELECTION,&save);
 OK(warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,8u,8u));
 OK(warship_battle_cancel(&b,WARSHIP_BATTLE_HUMAN));
 save=b;
 check_unchanged(&b,warship_battle_fire(&b,WARSHIP_BATTLE_HUMAN),WARSHIP_BATTLE_NO_SELECTION,&save);
 OK(warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,5u,0u));
 OK(warship_battle_select(&b,WARSHIP_BATTLE_CAPTAIN,0u,0u));
 WarshipBattlePublic public_before, public_after;
 OK(warship_battle_observe(&b,WARSHIP_BATTLE_CAPTAIN,&public_before));
 EQ(public_before.outgoing_count,0u);EQ(public_before.incoming_count,0u);
 OK(warship_battle_fire(&b,WARSHIP_BATTLE_HUMAN));
 EQ(warship_battle_phase(&b),WARSHIP_BATTLE_PREPARING);
 save=b;
 check_unchanged(&b,warship_battle_fire(&b,WARSHIP_BATTLE_HUMAN),WARSHIP_BATTLE_ALREADY_LOCKED,&save);
 check_unchanged(&b,warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,6u,0u),WARSHIP_BATTLE_ALREADY_LOCKED,&save);
 check_unchanged(&b,warship_battle_cancel(&b,WARSHIP_BATTLE_HUMAN),WARSHIP_BATTLE_ALREADY_LOCKED,&save);
 WarshipBattlePair pair;memset(&pair,0x3a,sizeof pair);WarshipBattlePair old_pair=pair;
 check_unchanged(&b,warship_battle_resolve(&b,&pair),WARSHIP_BATTLE_BAD_PHASE,&save);
 CHECK(memcmp(&pair,&old_pair,sizeof pair)==0);
 OK(warship_battle_observe(&b,WARSHIP_BATTLE_CAPTAIN,&public_after));
 CHECK(memcmp(&public_before,&public_after,sizeof public_before)==0); /* FEU not public */
 OK(warship_battle_fire(&b,WARSHIP_BATTLE_CAPTAIN));
 EQ(warship_battle_phase(&b),WARSHIP_BATTLE_READY);
 save=b;
 check_unchanged(&b,warship_battle_fire(&b,WARSHIP_BATTLE_HUMAN),WARSHIP_BATTLE_BAD_PHASE,&save);
 OK(warship_battle_resolve(&b,&pair));
 EQ(pair.round,1u);EQ(pair.verdict,WARSHIP_BATTLE_CONTINUE);
 EQ(pair.human.impact,WARSHIP_BATTLE_HIT);EQ(pair.human.ship_id,WARSHIP_BATTLE_NO_SHIP);
 EQ(pair.captain.impact,WARSHIP_BATTLE_HIT);EQ(pair.captain.ship_id,WARSHIP_BATTLE_NO_SHIP);
 EQ(warship_battle_phase(&b),WARSHIP_BATTLE_AWAIT_REVEAL);
 save=b;
 check_unchanged(&b,warship_battle_resolve(&b,&pair),WARSHIP_BATTLE_BAD_PHASE,&save);
 check_unchanged(&b,warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,7u,7u),WARSHIP_BATTLE_BAD_PHASE,&save);
 WarshipBattlePublic pub;OK(warship_battle_observe(&b,WARSHIP_BATTLE_CAPTAIN,&pub));
 EQ(pub.outgoing_count,1u); EQ(pub.incoming_count,1u);
 EQ(pub.outgoing[0].x,0u);EQ(pub.incoming[0].x,5u);
 EQ(pub.incoming[0].ship_id,WARSHIP_BATTLE_NO_SHIP);
 WarshipBattlePublic unchanged=pub;
 EQ(warship_battle_observe(&b,(WarshipBattleSide)2,&pub),WARSHIP_BATTLE_BAD_SIDE);
 CHECK(memcmp(&pub,&unchanged,sizeof pub)==0);
 EQ(warship_battle_observe(&b,WARSHIP_BATTLE_CAPTAIN,NULL),WARSHIP_BATTLE_BAD_ARGUMENT);
 OK(warship_battle_finish_reveal(&b));
 save=b;
 check_unchanged(&b,warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,5u,0u),WARSHIP_BATTLE_ALREADY_ATTACKED,&save);
 check_unchanged(&b,warship_battle_select(&b,WARSHIP_BATTLE_CAPTAIN,0u,0u),WARSHIP_BATTLE_ALREADY_ATTACKED,&save);
 check_unchanged(&b,warship_battle_finish_reveal(&b),WARSHIP_BATTLE_BAD_PHASE,&save);
}

static void lock_order(void)
{
 WarshipBattle a=new_battle(&fleet0,&fleet1);
 WarshipBattle b=new_battle(&fleet0,&fleet1);
 WarshipBattlePair p,q;
 OK(warship_battle_select(&a,WARSHIP_BATTLE_HUMAN,9u,9u));
 OK(warship_battle_select(&a,WARSHIP_BATTLE_CAPTAIN,9u,9u));
 OK(warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,9u,9u));
 OK(warship_battle_select(&b,WARSHIP_BATTLE_CAPTAIN,9u,9u));
 OK(warship_battle_fire(&a,WARSHIP_BATTLE_HUMAN));
 OK(warship_battle_fire(&a,WARSHIP_BATTLE_CAPTAIN));
 OK(warship_battle_fire(&b,WARSHIP_BATTLE_CAPTAIN));
 OK(warship_battle_fire(&b,WARSHIP_BATTLE_HUMAN));
 OK(warship_battle_resolve(&a,&p));OK(warship_battle_resolve(&b,&q));
 CHECK(memcmp(&p,&q,sizeof p)==0);
 CHECK(memcmp(&a,&b,sizeof a)==0);
 EQ(p.human.impact,WARSHIP_BATTLE_MISS);EQ(p.captain.impact,WARSHIP_BATTLE_MISS);
 EQ(p.human.ship_id,WARSHIP_BATTLE_NO_SHIP);
}

/* 17 hits in canonical (ship identity, cell) order; 17 different open-water
 * cells for an opponent that must never sink the other side. */
static void hit_at(unsigned ordinal, uint8_t x_offset, uint8_t *x, uint8_t *y)
{
 for(unsigned ship=0u;ship<5u;ship++){
  if(ordinal<ship_lengths[ship]) { *x=(uint8_t)(x_offset+ordinal);*y=(uint8_t)ship;return; }
  ordinal-=ship_lengths[ship];
 }
 abort();
}
static void water_at(unsigned ordinal, uint8_t *x, uint8_t *y)
{
 *x=(uint8_t)(ordinal%10u);
 *y=(uint8_t)(5u+ordinal/10u);
}

static void endings(unsigned mode)
{
 /* 1=victory 2=defeat 3=draw */
 WarshipBattle b=new_battle(&fleet0,&fleet1);
 for(unsigned n=0;n<17u;n++){
  uint8_t hx,hy,cx,cy;
  if(mode==1u || mode==3u)hit_at(n,5u,&hx,&hy);else water_at(n,&hx,&hy);
  if(mode==2u || mode==3u)hit_at(n,0u,&cx,&cy);else water_at(n,&cx,&cy);
  OK(warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,hx,hy));
  OK(warship_battle_select(&b,WARSHIP_BATTLE_CAPTAIN,cx,cy));
  if(n%2u==0u){OK(warship_battle_fire(&b,WARSHIP_BATTLE_HUMAN));OK(warship_battle_fire(&b,WARSHIP_BATTLE_CAPTAIN));}
  else{OK(warship_battle_fire(&b,WARSHIP_BATTLE_CAPTAIN));OK(warship_battle_fire(&b,WARSHIP_BATTLE_HUMAN));}
  WarshipBattlePair p;OK(warship_battle_resolve(&b,&p));
  EQ(p.round,n+1u);
  if(mode==1u || mode==3u) {
   unsigned cumulative=0;unsigned sunk=0;
   for(unsigned i=0;i<5u;i++){cumulative+=ship_lengths[i];if(n+1u==cumulative){sunk=1u;EQ(p.human.ship_id,i);}}
   EQ(p.human.impact,sunk?WARSHIP_BATTLE_SUNK:WARSHIP_BATTLE_HIT);
   if(!sunk)EQ(p.human.ship_id,WARSHIP_BATTLE_NO_SHIP);
  }else EQ(p.human.impact,WARSHIP_BATTLE_MISS);
  if(mode==2u || mode==3u){
   unsigned cumulative=0;unsigned sunk=0;
   for(unsigned i=0;i<5u;i++){cumulative+=ship_lengths[i];if(n+1u==cumulative){sunk=1u;EQ(p.captain.ship_id,i);}}
   EQ(p.captain.impact,sunk?WARSHIP_BATTLE_SUNK:WARSHIP_BATTLE_HIT);
  }else EQ(p.captain.impact,WARSHIP_BATTLE_MISS);
  if(n<16u){EQ(p.verdict,WARSHIP_BATTLE_CONTINUE);EQ(warship_battle_phase(&b),WARSHIP_BATTLE_AWAIT_REVEAL);OK(warship_battle_finish_reveal(&b));}
  else{
   EQ(p.verdict,mode==1u?WARSHIP_BATTLE_VICTORY:mode==2u?WARSHIP_BATTLE_DEFEAT:WARSHIP_BATTLE_DRAW);
   EQ(warship_battle_phase(&b),WARSHIP_BATTLE_FINISHED);
   WarshipBattle saved=b;
   check_unchanged(&b,warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,9u,9u),WARSHIP_BATTLE_BAD_PHASE,&saved);
   check_unchanged(&b,warship_battle_fire(&b,WARSHIP_BATTLE_HUMAN),WARSHIP_BATTLE_BAD_PHASE,&saved);
   check_unchanged(&b,warship_battle_resolve(&b,&p),WARSHIP_BATTLE_BAD_PHASE,&saved);
   check_unchanged(&b,warship_battle_finish_reveal(&b),WARSHIP_BATTLE_BAD_PHASE,&saved);
   WarshipBattleVerdict v=WARSHIP_BATTLE_CONTINUE;
   OK(warship_battle_take_final(&b,&v));EQ(v,p.verdict);
   saved=b;v=WARSHIP_BATTLE_CONTINUE;
   check_unchanged(&b,warship_battle_take_final(&b,&v),WARSHIP_BATTLE_NO_EVENT,&saved);
   EQ(v,WARSHIP_BATTLE_CONTINUE);
   WarshipBattlePublic pub;OK(warship_battle_observe(&b,WARSHIP_BATTLE_CAPTAIN,&pub));
   EQ(pub.outgoing_count,17u);EQ(pub.incoming_count,17u);EQ(pub.verdict,p.verdict);
  }
 }
}

/* Same visible CPU observations with different hidden human fleets.
 * Never pass WarshipBattle itself to future AI. No unpublished targeting
 * enters the public snapshot; known hits are unclassified until sinking. */
static void confidentiality(void)
{
 WarshipBattle a=new_battle(&fleet0,&fleet1),b=new_battle(&fleet1,&fleet1);
 WarshipBattlePublic pa,pb;
 OK(warship_battle_observe(&a,WARSHIP_BATTLE_CAPTAIN,&pa));
 OK(warship_battle_observe(&b,WARSHIP_BATTLE_CAPTAIN,&pb));
 CHECK(memcmp(&pa,&pb,sizeof pa)==0);
 OK(warship_battle_select(&a,WARSHIP_BATTLE_HUMAN,9u,9u));
 OK(warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,9u,9u));
 OK(warship_battle_select(&a,WARSHIP_BATTLE_CAPTAIN,9u,9u));
 OK(warship_battle_select(&b,WARSHIP_BATTLE_CAPTAIN,9u,9u));
 OK(warship_battle_fire(&a,WARSHIP_BATTLE_HUMAN));
 OK(warship_battle_fire(&b,WARSHIP_BATTLE_HUMAN));
 OK(warship_battle_observe(&a,WARSHIP_BATTLE_CAPTAIN,&pa));
 OK(warship_battle_observe(&b,WARSHIP_BATTLE_CAPTAIN,&pb));
 CHECK(memcmp(&pa,&pb,sizeof pa)==0);
 OK(warship_battle_fire(&a,WARSHIP_BATTLE_CAPTAIN));
 OK(warship_battle_fire(&b,WARSHIP_BATTLE_CAPTAIN));
 WarshipBattlePair p;
 OK(warship_battle_resolve(&a,&p));OK(warship_battle_resolve(&b,&p));
 OK(warship_battle_observe(&a,WARSHIP_BATTLE_CAPTAIN,&pa));
 OK(warship_battle_observe(&b,WARSHIP_BATTLE_CAPTAIN,&pb));
 CHECK(memcmp(&pa,&pb,sizeof pa)==0);
}


static void hundred_rounds(void)
{
 WarshipBattle b=new_battle(&fleet0,&fleet1);
 unsigned round=0u;
 /* Visit all 83 empty cells, then all 17 occupied cells for both sides. */
 for(unsigned y=0u;y<10u;y++) for(unsigned x=0u;x<10u;x++) {
  const int is_ship = y<5u && x<ship_lengths[y];
  if(is_ship) continue;
  uint8_t hx=(uint8_t)x, hy=(uint8_t)y;
  /* Captain has its fleet in columns 5..9; find a matching empty cell. */
  uint8_t cx=(uint8_t)((x+5u)%10u),cy=(uint8_t)y;
  OK(warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,cx,cy));
  OK(warship_battle_select(&b,WARSHIP_BATTLE_CAPTAIN,hx,hy));
  OK(warship_battle_fire(&b,WARSHIP_BATTLE_HUMAN));
  OK(warship_battle_fire(&b,WARSHIP_BATTLE_CAPTAIN));
  WarshipBattlePair p;OK(warship_battle_resolve(&b,&p));
  ++round;EQ(p.round,round);EQ(p.verdict,WARSHIP_BATTLE_CONTINUE);
  EQ(p.human.impact,WARSHIP_BATTLE_MISS);EQ(p.captain.impact,WARSHIP_BATTLE_MISS);
  OK(warship_battle_finish_reveal(&b));
 }
 EQ(round,83u);
 for(unsigned i=0u;i<17u;i++){
  uint8_t hx,hy,cx,cy;
  hit_at(i,5u,&hx,&hy);hit_at(i,0u,&cx,&cy);
  OK(warship_battle_select(&b,WARSHIP_BATTLE_HUMAN,hx,hy));
  OK(warship_battle_select(&b,WARSHIP_BATTLE_CAPTAIN,cx,cy));
  OK(warship_battle_fire(&b,WARSHIP_BATTLE_CAPTAIN));
  OK(warship_battle_fire(&b,WARSHIP_BATTLE_HUMAN));
  WarshipBattlePair p;OK(warship_battle_resolve(&b,&p));
  ++round;EQ(p.round,round);
  if(i<16u){EQ(p.verdict,WARSHIP_BATTLE_CONTINUE);OK(warship_battle_finish_reveal(&b));}
  else EQ(p.verdict,WARSHIP_BATTLE_DRAW);
 }
 EQ(round,100u);
 WarshipBattlePublic pub;OK(warship_battle_observe(&b,WARSHIP_BATTLE_CAPTAIN,&pub));
 EQ(pub.outgoing_count,100u);EQ(pub.incoming_count,100u);
}

static void hostile_input(void)
{
 WarshipBattle b=new_battle(&fleet0,&fleet1), before=b;
 EQ(warship_battle_select(NULL,WARSHIP_BATTLE_HUMAN,0u,0u),WARSHIP_BATTLE_BAD_ARGUMENT);
 EQ(warship_battle_fire(NULL,WARSHIP_BATTLE_HUMAN),WARSHIP_BATTLE_BAD_ARGUMENT);
 EQ(warship_battle_cancel(NULL,WARSHIP_BATTLE_HUMAN),WARSHIP_BATTLE_BAD_ARGUMENT);
 EQ(warship_battle_finish_reveal(NULL),WARSHIP_BATTLE_BAD_ARGUMENT);
 EQ(warship_battle_resolve(NULL,NULL),WARSHIP_BATTLE_BAD_ARGUMENT);
 WarshipBattlePublic pub;memset(&pub,0xa5,sizeof pub);
 EQ(warship_battle_observe(NULL,WARSHIP_BATTLE_HUMAN,&pub),WARSHIP_BATTLE_BAD_ARGUMENT);
 EQ(warship_battle_observe(&b,WARSHIP_BATTLE_HUMAN,NULL),WARSHIP_BATTLE_BAD_ARGUMENT);
 CHECK(memcmp(&before,&b,sizeof b)==0);
 WarshipBattleVerdict v=WARSHIP_BATTLE_CONTINUE;
 EQ(warship_battle_take_final(&b,&v),WARSHIP_BATTLE_NO_EVENT);
 EQ(v,WARSHIP_BATTLE_CONTINUE);
 EQ(warship_battle_take_final(&b,NULL),WARSHIP_BATTLE_BAD_ARGUMENT);
 EQ(warship_battle_resolve(&b,NULL),WARSHIP_BATTLE_BAD_ARGUMENT);
 WarshipBattle corrupted=b;corrupted.magic=0u;
 EQ(warship_battle_select(&corrupted,WARSHIP_BATTLE_HUMAN,0u,0u),WARSHIP_BATTLE_BAD_ARGUMENT);
 EQ(warship_battle_observe(&corrupted,WARSHIP_BATTLE_HUMAN,&pub),WARSHIP_BATTLE_BAD_ARGUMENT);
 EQ(warship_battle_phase(&corrupted),WARSHIP_BATTLE_FINISHED);
}

int main(void)
{
 invalid_init();selection_and_secrecy();lock_order();
 endings(1u);endings(2u);endings(3u);
 confidentiality();hundred_rounds();hostile_input();
 printf("D02 PASS sizeof(battle)=%zu sizeof(public)=%zu sizeof(pair)=%zu\n",sizeof(WarshipBattle),sizeof(WarshipBattlePublic),sizeof(WarshipBattlePair));
 return EXIT_SUCCESS;
}
