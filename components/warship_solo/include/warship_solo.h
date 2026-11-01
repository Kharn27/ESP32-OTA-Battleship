#ifndef WARSHIP_SOLO_H
#define WARSHIP_SOLO_H

#include <stdint.h>
#include "warship_battle.h"
#include "warship_captain.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WARSHIP_SOLO_OK = 0,
    WARSHIP_SOLO_BAD_ARGUMENT,
    WARSHIP_SOLO_BAD_FLEET,
    WARSHIP_SOLO_BAD_PHASE,
    WARSHIP_SOLO_OUT_OF_BOUNDS,
    WARSHIP_SOLO_ALREADY_ATTACKED,
    WARSHIP_SOLO_DUPLICATE_SELECTION,
    WARSHIP_SOLO_NO_SELECTION,
    WARSHIP_SOLO_ALREADY_FIRED,
    WARSHIP_SOLO_RANDOM_FAILURE,
    WARSHIP_SOLO_NO_EVENT,
    WARSHIP_SOLO_INTERNAL_ERROR
} WarshipSoloStatus;

typedef enum {
    WARSHIP_SOLO_SELECTING = 0,
    WARSHIP_SOLO_AWAIT_REVEAL = 1,
    WARSHIP_SOLO_FINISHED = 2
} WarshipSoloPhase;

/* Owned by the application/controller; contains BOTH SECRET fleets in D02.
 * Do not expose or copy this object to rendering/UI consumers. Its sole
 * sanctioned UI projection is warship_solo_observe().
 * Caller provides storage; no heap, OS, timer or display dependency.
 */
typedef struct {
    WarshipBattle battle;
    uint32_t magic;
    uint8_t phase;
} WarshipSoloSession;

/* Public human-facing snapshot. No private fleets, ownership maps, CPU
 * target waiting to fire or unrevealed shots. The latest pair is deliberately
 * hidden here until warship_solo_ack_reveal(), including terminal verdict.
 * The pair returned by warship_solo_fire() is for the presentation controller
 * to reveal both outcomes at the SAME visual instant.
 */
typedef struct {
    WarshipBattlePublic history;
    uint8_t phase;
    uint8_t has_selection;
    uint8_t target_x;
    uint8_t target_y;
} WarshipSoloPublic;

/* Initialize ONE solo session from two valid, separately owned fleets.
 * Only success replaces *session. Inputs must not overlap destination.
 * A fresh session starts in SELECTING (D02 PREPARING); deployment is D07.
 */
WarshipSoloStatus warship_solo_init(WarshipSoloSession *session,
                                    const WarshipFleet *human,
                                    const WarshipFleet *captain);

/* Human controls only, while SELECTING. Same-target select is rejected;
 * selecting a different unknown target replaces the previous one.
 * Errors leave the session unchanged.
 */
WarshipSoloStatus warship_solo_select(WarshipSoloSession *session,
                                      uint8_t x, uint8_t y);
WarshipSoloStatus warship_solo_cancel(WarshipSoloSession *session);

/* One human FEU -> CPU decision on strictly CAPTAIN's public D02 view ->
 * lock BOTH selections -> exactly ONE D02 simultaneous resolution.
 * The entire transaction commits only on success: on RNG/error, session
 * and *pair are byte-for-byte unchanged; human may retry FEU.
 * Exactly one CPU RNG callback on accepted FEU; none on validation errors.
 * Success enters AWAIT_REVEAL, including the final round. Repeated FEU
 * there returns ALREADY_FIRED. The pair is public outcome data, NOT a
 * secretly known fleet/wreck map. Caller keeps it until visual revelation.
 */
WarshipSoloStatus warship_solo_fire(WarshipSoloSession *session,
                                    WarshipCaptainRandom rng,
                                    void *rng_context,
                                    WarshipBattlePair *pair);

/* Called exactly once by presentation AFTER showing BOTH results.
 * If game continues: D02 finish_reveal -> SELECTING immediately.
 * If terminal: FINISHED, enabling take_final; no further targeting.
 */
WarshipSoloStatus warship_solo_ack_reveal(WarshipSoloSession *session);

/* Human-facing, non-secret snapshot. While awaiting reveal, excludes the
 * newest pair and its final verdict. Never exposes hidden ship positions.
 * On failure the output is byte-for-byte unchanged.
 */
WarshipSoloStatus warship_solo_observe(const WarshipSoloSession *session,
                                       WarshipSoloPublic *out);

/* Terminal event once, only AFTER the terminal pair's reveal was
 * acknowledged. D02 owns one-shot final event semantics; second call
 * yields NO_EVENT. On failure, *out is unchanged.
 */
WarshipSoloStatus warship_solo_take_final(WarshipSoloSession *session,
                                          WarshipBattleVerdict *out);

#ifdef __cplusplus
}
#endif
#endif
