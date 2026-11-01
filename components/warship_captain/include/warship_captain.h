#ifndef WARSHIP_CAPTAIN_H
#define WARSHIP_CAPTAIN_H

#include <stdint.h>
#include "warship_battle.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WARSHIP_CAPTAIN_OK = 0,
    WARSHIP_CAPTAIN_BAD_ARGUMENT,
    WARSHIP_CAPTAIN_BAD_OBSERVATION,
    WARSHIP_CAPTAIN_NO_TARGET,
    WARSHIP_CAPTAIN_RANDOM_FAILURE
} WarshipCaptainStatus;

typedef struct {
    uint8_t x;
    uint8_t y;
} WarshipCaptainTarget;

/* Zero for success and a 32-bit sample written to *value; nonzero on failure.
 * The caller manages RNG context. Failure leaves target unmodified.
 */
typedef int (*WarshipCaptainRandom)(void *context, uint32_t *value);

/* D03/D04/D05: choose an untried target from CAPTAIN's D02 public observation.
 * The caller MUST supply warship_battle_observe(battle, CAPTAIN, &view),
 * NOT the private WarshipBattle or either WarshipFleet.
 * This API has no persistent state, heap, IO, timers or hidden data.
 * Exactly one RNG callback is invoked on a valid ongoing observation
 * containing at least one untried cell, even if the target is forced.
 * Valid pointers must identify nonoverlapping objects; caller retains all.
 * On every non-OK status, *out is byte-for-byte unchanged.
 * NO_TARGET: terminal verdict or all 100 outgoing cells already attacked.
 * D04 reconstructs several public HIT tracks from history. Contradicted
 * alignments are revised; SUNK proximity is only a soft priority hint,
 * NEVER an ownership attribution or a halo of excluded water. No persistent
 * CPU state is needed. D05 filters only general exploration using public
 * SUNK types and straight-line plausible runs, without assigning old HITs.
 * Execution is bounded by small, fixed scans of the 10x10 grid.
 */
WarshipCaptainStatus warship_captain_choose(const WarshipBattlePublic *view,
                                            WarshipCaptainRandom random_u32,
                                            void *random_context,
                                            WarshipCaptainTarget *out);

#ifdef __cplusplus
}
#endif
#endif
