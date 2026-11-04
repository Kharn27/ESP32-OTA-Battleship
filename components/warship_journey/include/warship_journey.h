#ifndef WARSHIP_JOURNEY_H
#define WARSHIP_JOURNEY_H

#include "warship_combat_ui.h"
#include "warship_event_bridge.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WARSHIP_JOURNEY_DONE=0,
    WARSHIP_JOURNEY_NOT_HANDLED,
    WARSHIP_JOURNEY_IGNORED,
    WARSHIP_JOURNEY_SCORE_UNCONFIRMED,
    WARSHIP_JOURNEY_GAME_ERROR,
    WARSHIP_JOURNEY_BAD_ARGUMENT
} WarshipJourneyStatus;

/* Exactly one synchronous call for a confirmed, D07-emitted FORFEIT.
 * D11's existing mono-writer reserves IDs and performs the NVS commit.
 * Return the ACTUAL status, never convert uncertainty into success. */
typedef WarshipEventStatus (*WarshipJourneyPersist)(
    void *context, WarshipLifecycleEvent event);

/* D12 routes COMBAT/CANCEL as QUITTER, CONFIRM/FIRE as CONTINUER,
 * CONFIRM/CANCEL as ABANDONNER, RESULT/FIRE as REJOUER,
 * RESULT/CANCEL as ACCUEIL; RESULT/SWITCH remains D10 inspection.
 * Any non-navigation D10 action is NOT_HANDLED for existing D10 path.
 * The lifecycle stays private to controller, never raster.
 * Output status is changed only after a successfully recognized command.
 */
WarshipJourneyStatus warship_journey_apply(
    WarshipLifecycle *game, WarshipCombatUi *ui, WarshipCombatHit hit,
    WarshipFleetRandom captain_rng, void *captain_ctx,
    WarshipJourneyPersist persist, void *persist_ctx,
    WarshipEventStatus *ledger_status);

/* Consume exactly one B01 gesture via the existing D10 debouncer, but
 * recognize the same technical hitboxes in both ATTAQUE and DÉFENSE,
 * and in CONFIRM/RESULT. The journey dispatcher gates actions by D07
 * phase and by pending FEU. A held/invalid/bouncing gesture is never
 * deferred into a new phase.
 */
WarshipCombatStatus warship_journey_input_step(
    WarshipCombatInput *input, const WarshipCombatUi *ui,
    uint8_t down, uint8_t valid, uint16_t x, uint16_t y,
    uint64_t now_ms, WarshipCombatHit *hit);

#ifdef __cplusplus
}
#endif
#endif
