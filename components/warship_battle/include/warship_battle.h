#ifndef WARSHIP_BATTLE_H
#define WARSHIP_BATTLE_H

#include <stdint.h>
#include "warship_fleet.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WARSHIP_BATTLE_CELLS (WARSHIP_FLEET_GRID * WARSHIP_FLEET_GRID)
#define WARSHIP_BATTLE_NO_SHIP UINT8_MAX

typedef enum {
    WARSHIP_BATTLE_HUMAN = 0,
    WARSHIP_BATTLE_CAPTAIN = 1,
    WARSHIP_BATTLE_SIDES = 2
} WarshipBattleSide;

typedef enum {
    WARSHIP_BATTLE_PREPARING = 0,
    WARSHIP_BATTLE_READY = 1,
    WARSHIP_BATTLE_AWAIT_REVEAL = 2,
    WARSHIP_BATTLE_FINISHED = 3
} WarshipBattlePhase;

typedef enum {
    WARSHIP_BATTLE_MISS = 0,
    WARSHIP_BATTLE_HIT = 1,
    WARSHIP_BATTLE_SUNK = 2
} WarshipBattleImpact;

typedef enum {
    WARSHIP_BATTLE_CONTINUE = 0,
    WARSHIP_BATTLE_VICTORY = 1,
    WARSHIP_BATTLE_DEFEAT = 2,
    WARSHIP_BATTLE_DRAW = 3
} WarshipBattleVerdict;

typedef enum {
    WARSHIP_BATTLE_OK = 0,
    WARSHIP_BATTLE_BAD_ARGUMENT,
    WARSHIP_BATTLE_BAD_FLEET,
    WARSHIP_BATTLE_BAD_SIDE,
    WARSHIP_BATTLE_OUT_OF_BOUNDS,
    WARSHIP_BATTLE_ALREADY_ATTACKED,
    WARSHIP_BATTLE_DUPLICATE_SELECTION,
    WARSHIP_BATTLE_NO_SELECTION,
    WARSHIP_BATTLE_ALREADY_LOCKED,
    WARSHIP_BATTLE_BAD_PHASE,
    WARSHIP_BATTLE_NO_EVENT
} WarshipBattleStatus;

/* Only SUNK has a valid ship_id (0..4); MISS and HIT use NO_SHIP.
 * No hit or sunk record identifies earlier squares belonging to a ship. */
typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t impact;
    uint8_t ship_id;
} WarshipBattleShot;

typedef struct {
    WarshipBattleShot human;   /* Human fires on Captain. */
    WarshipBattleShot captain; /* Captain fires on human. */
    uint8_t round;             /* 1..100; completed round count. */
    uint8_t verdict;           /* Human-centric, after BOTH shots. */
} WarshipBattlePair;

/* Safe-to-pass-by-value CPU observation. Contains no fleet, occupancy
 * map, ship hit counters, pending targets or opponent lock information.
 * Lists contain ONLY completed attacks, in chronological order.
 * Use this value, NEVER a WarshipBattle pointer, for the future CPU.
 */
typedef struct {
    WarshipBattleShot outgoing[WARSHIP_BATTLE_CELLS];
    WarshipBattleShot incoming[WARSHIP_BATTLE_CELLS];
    uint8_t outgoing_count;
    uint8_t incoming_count;
    uint8_t verdict;
} WarshipBattlePublic;

/* ENGINE-PRIVATE DATA: the entire state includes both SECRET fleets.
 * The session owns this fixed-size object; NEVER expose it to CPU AI.
 * Do not mutate any fields except through the API. No allocation/heap.
 */
typedef struct {
    WarshipFleet fleets[WARSHIP_BATTLE_SIDES];
    WarshipBattleShot history[WARSHIP_BATTLE_SIDES][WARSHIP_BATTLE_CELLS];
    uint8_t attacked[WARSHIP_BATTLE_SIDES][WARSHIP_BATTLE_CELLS];
    uint8_t hit_by_ship[WARSHIP_BATTLE_SIDES][WARSHIP_FLEET_SHIPS];
    uint8_t total_hits[WARSHIP_BATTLE_SIDES];
    uint8_t counts[WARSHIP_BATTLE_SIDES];
    uint8_t target_x[WARSHIP_BATTLE_SIDES];
    uint8_t target_y[WARSHIP_BATTLE_SIDES];
    uint8_t selected[WARSHIP_BATTLE_SIDES];
    uint8_t locked[WARSHIP_BATTLE_SIDES];
    uint8_t phase;
    uint8_t verdict;
    uint8_t final_event_pending;
    uint32_t magic;
} WarshipBattle;

/* All outputs are caller-owned, nonoverlapping objects. For each failing
 * call, the battle and all output parameters remain byte-for-byte unchanged.
 * Initialize with two independently valid D01 fleets. Success replaces the
 * entire battle state, copying both fleets. Caller changes to originals
 * after init do not affect the battle. Init failure preserves destination. */
WarshipBattleStatus warship_battle_init(WarshipBattle *battle,
                                        const WarshipFleet *human,
                                        const WarshipFleet *captain);

/* Selection is replaceable until FEU; same selection is rejected as
 * DUPLICATE_SELECTION. Cancel explicitly clears it. Neither affects
 * opponent selection. Coordinates x,y are 0..9. */
WarshipBattleStatus warship_battle_select(WarshipBattle *battle,
                                          WarshipBattleSide side,
                                          uint8_t x, uint8_t y);
WarshipBattleStatus warship_battle_cancel(WarshipBattle *battle,
                                          WarshipBattleSide side);
/* FEU irreversibly locks that side. Second FEU returns ALREADY_LOCKED.
 * A lock NEVER resolves nor reveals a shot. Second lock makes READY. */
WarshipBattleStatus warship_battle_fire(WarshipBattle *battle,
                                        WarshipBattleSide side);
/* Only READY permits resolution. Commits BOTH shots and publishes a pair
 * in ONE operation, regardless of locking order. Cannot run twice.
 * Continue -> AWAIT_REVEAL, final verdict -> FINISHED. */
WarshipBattleStatus warship_battle_resolve(WarshipBattle *battle,
                                           WarshipBattlePair *pair);
/* Called by session only AFTER visual revelation (no timers/buttons here).
 * Reopens selection for next round if no final verdict. */
WarshipBattleStatus warship_battle_finish_reveal(WarshipBattle *battle);
/* One-shot terminal event for future palmares: ONCE per finished game.
 * Repeated reads report NO_EVENT and leave output unchanged. */
WarshipBattleStatus warship_battle_take_final(WarshipBattle *battle,
                                              WarshipBattleVerdict *verdict);
/* Public snapshot for one side; NEVER exposes hidden placements. Valid
 * after init, even before two locks (no unpublished result leaks). */
WarshipBattleStatus warship_battle_observe(const WarshipBattle *battle,
                                           WarshipBattleSide observer,
                                           WarshipBattlePublic *out);
WarshipBattlePhase warship_battle_phase(const WarshipBattle *battle);

#ifdef __cplusplus
}
#endif
#endif
