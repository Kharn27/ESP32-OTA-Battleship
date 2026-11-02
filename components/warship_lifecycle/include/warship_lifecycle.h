#ifndef WARSHIP_LIFECYCLE_H
#define WARSHIP_LIFECYCLE_H

#include <stdint.h>
#include "warship_solo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WARSHIP_LIFECYCLE_OK = 0,
    WARSHIP_LIFECYCLE_BAD_ARGUMENT,
    WARSHIP_LIFECYCLE_BAD_PHASE,
    WARSHIP_LIFECYCLE_RANDOM_FAILURE,
    WARSHIP_LIFECYCLE_NO_SELECTION,
    WARSHIP_LIFECYCLE_OUT_OF_BOUNDS,
    WARSHIP_LIFECYCLE_ALREADY_ATTACKED,
    WARSHIP_LIFECYCLE_DUPLICATE_SELECTION,
    WARSHIP_LIFECYCLE_ALREADY_FIRED,
    WARSHIP_LIFECYCLE_REVEAL_DENIED,
    WARSHIP_LIFECYCLE_INTERNAL_ERROR
} WarshipLifecycleStatus;

typedef enum {
    WARSHIP_LIFECYCLE_HOME = 0,
    WARSHIP_LIFECYCLE_DEPLOY = 1,
    WARSHIP_LIFECYCLE_COMBAT = 2,
    WARSHIP_LIFECYCLE_CONFIRM_ABANDON = 3,
    WARSHIP_LIFECYCLE_RESULT = 4
} WarshipLifecyclePhase;

/* Events are values for the future D08 consumer, NOT counters. Each
 * successful transition emits ONE event through its output parameter.
 * NONE is used on normal nonterminal acknowledgments / CONTINUER.
 * Delivery is synchronous: caller persists it before discarding the value.
 */
typedef enum {
    WARSHIP_LIFECYCLE_EVENT_NONE = 0,
    WARSHIP_LIFECYCLE_EVENT_VICTORY,
    WARSHIP_LIFECYCLE_EVENT_DEFEAT,
    WARSHIP_LIFECYCLE_EVENT_DRAW,
    WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT
} WarshipLifecycleEvent;

/* CONTROLLER PRIVATE: do NOT give this object to UI or Capitaine.
 * It contains human/captain placements and D06's secret session. No heap.
 * Callers retain stable storage, never copy it into public/render data.
 */
typedef struct {
    WarshipSoloSession solo;
    WarshipFleet human;
    WarshipFleet captain;
    uint32_t magic;
    uint8_t phase;
    uint8_t verdict;
    uint8_t reveal_requested;
    uint8_t reveal_visible;
} WarshipLifecycle;

/* UI-only projection. Human fleet is always available outside HOME
 * (including deployment/defense), never the captain's hidden fleet.
 * 'battle' is populated only after PRÊT, using D06's public snapshot;
 * D06 hides the latest shots until its reveal ACK.
 */
typedef struct {
    WarshipFleet human;
    WarshipSoloPublic battle;
    uint8_t phase;
    uint8_t has_human_fleet;
    uint8_t has_battle;
    uint8_t can_reveal_captain;
    uint8_t reveal_requested;
    uint8_t reveal_visible;
} WarshipLifecyclePublic;

/* All pointers to distinct caller-owned objects; RNG contexts must not
 * alias/mutate the controller. All errors preserve controller/output bytes.
 * init() creates HOME; start() may run only from HOME. It generates BOTH
 * fleets independently via D01, using separate injectable RNGs/contexts.
 */
WarshipLifecycleStatus warship_lifecycle_init(WarshipLifecycle *game);
WarshipLifecycleStatus warship_lifecycle_start(WarshipLifecycle *game,
                                               WarshipFleetRandom human_rng,
                                               void *human_context,
                                               WarshipFleetRandom captain_rng,
                                               void *captain_context);
/* Deployment is populated already. AUTO replaces only human fleet.
 * Matching generated formations are valid; AUTO never starts battle.
 */
WarshipLifecycleStatus warship_lifecycle_auto(WarshipLifecycle *game,
                                              WarshipFleetRandom human_rng,
                                              void *human_context);
WarshipLifecycleStatus warship_lifecycle_ready(WarshipLifecycle *game);
WarshipLifecycleStatus warship_lifecycle_leave_deploy(WarshipLifecycle *game);

/* D06-only combat controls; disallow actions in confirmation / result /
 * deployment and refuse leaving the FEU reveal phase by any other route.
 */
WarshipLifecycleStatus warship_lifecycle_select(WarshipLifecycle *game,
                                                uint8_t x, uint8_t y);
WarshipLifecycleStatus warship_lifecycle_cancel(WarshipLifecycle *game);
WarshipLifecycleStatus warship_lifecycle_fire(WarshipLifecycle *game,
                                              WarshipCaptainRandom rng,
                                              void *rng_context,
                                              WarshipBattlePair *pair);
/* After the presentation reveals BOTH shots, ACK the D06 pair. Emits
 * exactly one outcome event on the terminal round; otherwise NONE.
 * Result is not published until ACK. Never emit a second outcome.
 */
WarshipLifecycleStatus warship_lifecycle_ack_reveal(
    WarshipLifecycle *game, WarshipLifecycleEvent *event);

/* Only after PRÊT, before verdict, and not during FEU/animation.
 * request -> CONTINUER (abandon=0, NONE) OR confirmed ABANDONNER
 * (abandon=1, one global voluntary-forfeit event, HOME).
 * Invalid decision/phase preserves state and outputs.
 */
WarshipLifecycleStatus warship_lifecycle_request_abandon(
    WarshipLifecycle *game);
WarshipLifecycleStatus warship_lifecycle_decide_abandon(
    WarshipLifecycle *game, uint8_t abandon, WarshipLifecycleEvent *event);

/* RESULT only, after final reveal; REJOUER regenerates CPU fleet via D01,
 * reuses previous human formation and returns to DEPLOY (new PRÊT required).
 * Exit to HOME after result does not emit a forfeit.
 */
WarshipLifecycleStatus warship_lifecycle_replay(
    WarshipLifecycle *game, WarshipFleetRandom captain_rng,
    void *captain_context);
WarshipLifecycleStatus warship_lifecycle_leave_result(
    WarshipLifecycle *game);

WarshipLifecycleStatus warship_lifecycle_observe(
    const WarshipLifecycle *game, WarshipLifecyclePublic *out);

/* Post-DEFEAT only, opt-in controlled show/hide. Successful show authorizes
 * future toggling, but only 'visible' makes fleet read accessible.
 * The caller must clear its presentation cache on HIDE / leave / replay;
 * previously delivered copies cannot be remotely revoked.
 */
WarshipLifecycleStatus warship_lifecycle_set_captain_visible(
    WarshipLifecycle *game, uint8_t visible);
WarshipLifecycleStatus warship_lifecycle_copy_revealed_captain(
    const WarshipLifecycle *game, WarshipFleet *out);

#ifdef __cplusplus
}
#endif
#endif
