#include "warship_journey.h"

static int confirmed(WarshipEventStatus status)
{
    return status==WARSHIP_EVENT_APPLIED ||
           status==WARSHIP_EVENT_ALREADY_APPLIED;
}
WarshipCombatStatus warship_journey_input_step(
    WarshipCombatInput *in, const WarshipCombatUi *ui,
    uint8_t down, uint8_t valid, uint16_t x, uint16_t y,
    uint64_t ms, WarshipCombatHit *out)
{
    if (in==NULL || ui==NULL || out==NULL)
        return WARSHIP_COMBAT_BAD_ARGUMENT;
    /* Only geometry detection uses ATTACK; D12/D10 dispatchers guard
     * actual view+phase. This keeps CANCEL reachable from DEFENSE and
     * both result/confirmation controls reachable without a second
     * gesture state machine. A CELL in DEFENSE is rejected by D10. */
    return warship_combat_input_step(in,down,valid,x,y,ms,
                                     ui->pending,WARSHIP_COMBAT_ATTACK,out);
}
WarshipJourneyStatus warship_journey_apply(
    WarshipLifecycle *game, WarshipCombatUi *ui, WarshipCombatHit hit,
    WarshipFleetRandom captain_rng, void *captain_ctx,
    WarshipJourneyPersist persist, void *persist_ctx,
    WarshipEventStatus *ledger_status)
{
    if (game==NULL || ui==NULL || ledger_status==NULL ||
        hit.command>WARSHIP_COMBAT_INSPECT)
        return WARSHIP_JOURNEY_BAD_ARGUMENT;
    /* Separate D13 handler intercepts this gesture in the same B01
     * input stream. No D12 navigation or NVS side effect. */
    if (hit.command==WARSHIP_COMBAT_INSPECT)
        return WARSHIP_JOURNEY_NOT_HANDLED;
    if (hit.command==WARSHIP_COMBAT_NO_HIT)
        return WARSHIP_JOURNEY_IGNORED;
    /* No transition during an unresolved pair, even if the touch
     * dispatcher were bypassed in a unit test. */
    if (ui->pending)
        return WARSHIP_JOURNEY_IGNORED;

    WarshipLifecycleStatus s;
    WarshipLifecycleEvent emitted=WARSHIP_LIFECYCLE_EVENT_NONE;
    if (game->phase==WARSHIP_LIFECYCLE_COMBAT) {
        if (hit.command!=WARSHIP_COMBAT_CANCEL)
            return WARSHIP_JOURNEY_NOT_HANDLED;
        s=warship_lifecycle_request_abandon(game);
        if (s!=WARSHIP_LIFECYCLE_OK)
            return s==WARSHIP_LIFECYCLE_BAD_PHASE?
                WARSHIP_JOURNEY_IGNORED:WARSHIP_JOURNEY_GAME_ERROR;
        *ledger_status=WARSHIP_EVENT_NO_CHANGE;
        return WARSHIP_JOURNEY_DONE;
    }
    if (game->phase==WARSHIP_LIFECYCLE_CONFIRM_ABANDON) {
        if (hit.command!=WARSHIP_COMBAT_FIRE &&
            hit.command!=WARSHIP_COMBAT_CANCEL)
            return WARSHIP_JOURNEY_IGNORED;
        if (hit.command==WARSHIP_COMBAT_CANCEL && persist==NULL)
            return WARSHIP_JOURNEY_BAD_ARGUMENT;
        const uint8_t abandon=(uint8_t)(
            hit.command==WARSHIP_COMBAT_CANCEL);
        s=warship_lifecycle_decide_abandon(game,abandon,&emitted);
        if (s!=WARSHIP_LIFECYCLE_OK) return WARSHIP_JOURNEY_GAME_ERROR;
        if (!abandon) {
            *ledger_status=WARSHIP_EVENT_NO_CHANGE;
            return WARSHIP_JOURNEY_DONE;
        }
        /* D07 already went HOME; NVS error cannot fabricate a result
         * or rewind a confirmed voluntary action. D11 keeps pending. */
        warship_combat_init(ui,NULL,0u);
        *ledger_status=persist(persist_ctx,emitted);
        return confirmed(*ledger_status)?WARSHIP_JOURNEY_DONE:
               WARSHIP_JOURNEY_SCORE_UNCONFIRMED;
    }
    if (game->phase==WARSHIP_LIFECYCLE_RESULT) {
        if (hit.command==WARSHIP_COMBAT_SWITCH)
            return WARSHIP_JOURNEY_NOT_HANDLED;
        if (hit.command==WARSHIP_COMBAT_FIRE) {
            if (captain_rng==NULL) return WARSHIP_JOURNEY_BAD_ARGUMENT;
            s=warship_lifecycle_replay(game,captain_rng,captain_ctx);
        } else if (hit.command==WARSHIP_COMBAT_CANCEL) {
            s=warship_lifecycle_leave_result(game);
        } else return WARSHIP_JOURNEY_IGNORED;
        if (s!=WARSHIP_LIFECYCLE_OK) return WARSHIP_JOURNEY_GAME_ERROR;
        warship_combat_init(ui,NULL,0u);
        *ledger_status=WARSHIP_EVENT_NO_CHANGE;
        return WARSHIP_JOURNEY_DONE;
    }
    return WARSHIP_JOURNEY_IGNORED;
}
