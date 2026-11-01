#include "warship_solo.h"

#include <string.h>

#define SOLO_MAGIC UINT32_C(0x53303631)

static int valid(const WarshipSoloSession *session)
{
    if (session == NULL || session->magic != SOLO_MAGIC ||
        session->phase > WARSHIP_SOLO_FINISHED)
        return 0;
    const WarshipBattlePhase phase = warship_battle_phase(&session->battle);
    if (session->phase == WARSHIP_SOLO_SELECTING)
        return phase == WARSHIP_BATTLE_PREPARING;
    if (session->phase == WARSHIP_SOLO_AWAIT_REVEAL)
        return phase == WARSHIP_BATTLE_AWAIT_REVEAL ||
               phase == WARSHIP_BATTLE_FINISHED;
    return phase == WARSHIP_BATTLE_FINISHED;
}

static WarshipSoloStatus map_battle(WarshipBattleStatus status)
{
    switch (status) {
    case WARSHIP_BATTLE_OK: return WARSHIP_SOLO_OK;
    case WARSHIP_BATTLE_BAD_FLEET: return WARSHIP_SOLO_BAD_FLEET;
    case WARSHIP_BATTLE_OUT_OF_BOUNDS: return WARSHIP_SOLO_OUT_OF_BOUNDS;
    case WARSHIP_BATTLE_ALREADY_ATTACKED: return WARSHIP_SOLO_ALREADY_ATTACKED;
    case WARSHIP_BATTLE_DUPLICATE_SELECTION:
        return WARSHIP_SOLO_DUPLICATE_SELECTION;
    case WARSHIP_BATTLE_NO_SELECTION: return WARSHIP_SOLO_NO_SELECTION;
    case WARSHIP_BATTLE_BAD_PHASE: return WARSHIP_SOLO_BAD_PHASE;
    case WARSHIP_BATTLE_BAD_ARGUMENT: return WARSHIP_SOLO_BAD_ARGUMENT;
    default: return WARSHIP_SOLO_INTERNAL_ERROR;
    }
}

WarshipSoloStatus warship_solo_init(WarshipSoloSession *session,
                                    const WarshipFleet *human,
                                    const WarshipFleet *captain)
{
    if (session == NULL || human == NULL || captain == NULL)
        return WARSHIP_SOLO_BAD_ARGUMENT;
    WarshipSoloSession next;
    memset(&next, 0, sizeof next);
    const WarshipBattleStatus result =
        warship_battle_init(&next.battle, human, captain);
    if (result != WARSHIP_BATTLE_OK) return map_battle(result);
    next.magic = SOLO_MAGIC;
    next.phase = WARSHIP_SOLO_SELECTING;
    *session = next;
    return WARSHIP_SOLO_OK;
}

WarshipSoloStatus warship_solo_select(WarshipSoloSession *session,
                                      uint8_t x, uint8_t y)
{
    if (!valid(session)) return WARSHIP_SOLO_BAD_ARGUMENT;
    if (session->phase != WARSHIP_SOLO_SELECTING)
        return WARSHIP_SOLO_BAD_PHASE;
    return map_battle(warship_battle_select(&session->battle,
                                            WARSHIP_BATTLE_HUMAN, x, y));
}

WarshipSoloStatus warship_solo_cancel(WarshipSoloSession *session)
{
    if (!valid(session)) return WARSHIP_SOLO_BAD_ARGUMENT;
    if (session->phase != WARSHIP_SOLO_SELECTING)
        return WARSHIP_SOLO_BAD_PHASE;
    return map_battle(warship_battle_cancel(&session->battle,
                                            WARSHIP_BATTLE_HUMAN));
}

WarshipSoloStatus warship_solo_fire(WarshipSoloSession *session,
                                    WarshipCaptainRandom rng,
                                    void *rng_context,
                                    WarshipBattlePair *pair)
{
    if (!valid(session) || rng == NULL || pair == NULL)
        return WARSHIP_SOLO_BAD_ARGUMENT;
    if (session->phase == WARSHIP_SOLO_AWAIT_REVEAL)
        return WARSHIP_SOLO_ALREADY_FIRED;
    if (session->phase != WARSHIP_SOLO_SELECTING)
        return WARSHIP_SOLO_BAD_PHASE;
    if (session->battle.selected[WARSHIP_BATTLE_HUMAN] == 0u)
        return WARSHIP_SOLO_NO_SELECTION;

    /* Work on temporary state. Neither an RNG failure nor a downstream
     * engine failure can leave a locked human attack or a partial pair.
     * The CPU sees an observation BEFORE ANY fire/lock of this round.
     * In particular, the pending HUMAN target is not in this snapshot.
     */
    WarshipBattle staged = session->battle;
    WarshipBattlePublic public_cpu;
    if (warship_battle_observe(&staged, WARSHIP_BATTLE_CAPTAIN,
                               &public_cpu) != WARSHIP_BATTLE_OK)
        return WARSHIP_SOLO_INTERNAL_ERROR;
    WarshipCaptainTarget target;
    const WarshipCaptainStatus cpu =
        warship_captain_choose(&public_cpu, rng, rng_context, &target);
    if (cpu == WARSHIP_CAPTAIN_RANDOM_FAILURE)
        return WARSHIP_SOLO_RANDOM_FAILURE;
    if (cpu != WARSHIP_CAPTAIN_OK) return WARSHIP_SOLO_INTERNAL_ERROR;

    if (warship_battle_select(&staged, WARSHIP_BATTLE_CAPTAIN,
                              target.x, target.y) != WARSHIP_BATTLE_OK ||
        warship_battle_fire(&staged, WARSHIP_BATTLE_HUMAN) !=
                              WARSHIP_BATTLE_OK ||
        warship_battle_fire(&staged, WARSHIP_BATTLE_CAPTAIN) !=
                              WARSHIP_BATTLE_OK)
        return WARSHIP_SOLO_INTERNAL_ERROR;

    WarshipBattlePair resolved;
    if (warship_battle_resolve(&staged, &resolved) != WARSHIP_BATTLE_OK)
        return WARSHIP_SOLO_INTERNAL_ERROR;

    session->battle = staged;
    session->phase = WARSHIP_SOLO_AWAIT_REVEAL;
    *pair = resolved;
    return WARSHIP_SOLO_OK;
}

WarshipSoloStatus warship_solo_ack_reveal(WarshipSoloSession *session)
{
    if (!valid(session)) return WARSHIP_SOLO_BAD_ARGUMENT;
    if (session->phase != WARSHIP_SOLO_AWAIT_REVEAL)
        return WARSHIP_SOLO_BAD_PHASE;
    if (warship_battle_phase(&session->battle) == WARSHIP_BATTLE_FINISHED) {
        /* D02 has already resolved BOTH terminal shots atomically. */
        session->phase = WARSHIP_SOLO_FINISHED;
        return WARSHIP_SOLO_OK;
    }
    if (warship_battle_finish_reveal(&session->battle) != WARSHIP_BATTLE_OK)
        return WARSHIP_SOLO_INTERNAL_ERROR;
    session->phase = WARSHIP_SOLO_SELECTING;
    return WARSHIP_SOLO_OK;
}

WarshipSoloStatus warship_solo_observe(const WarshipSoloSession *session,
                                       WarshipSoloPublic *out)
{
    if (!valid(session) || out == NULL) return WARSHIP_SOLO_BAD_ARGUMENT;
    WarshipSoloPublic public_state;
    memset(&public_state, 0, sizeof public_state);
    if (warship_battle_observe(&session->battle, WARSHIP_BATTLE_HUMAN,
                               &public_state.history) != WARSHIP_BATTLE_OK)
        return WARSHIP_SOLO_INTERNAL_ERROR;
    public_state.phase = session->phase;
    if (session->phase == WARSHIP_SOLO_SELECTING &&
        session->battle.selected[WARSHIP_BATTLE_HUMAN] != 0u) {
        public_state.has_selection = 1u;
        public_state.target_x = session->battle.target_x[WARSHIP_BATTLE_HUMAN];
        public_state.target_y = session->battle.target_y[WARSHIP_BATTLE_HUMAN];
    }
    if (session->phase == WARSHIP_SOLO_AWAIT_REVEAL) {
        /* Resolved but NOT YET publicly revealed by the presentation.
         * Hide both new shots (and terminal outcome) in this projection.
         */
        if (public_state.history.outgoing_count == 0u ||
            public_state.history.incoming_count == 0u)
            return WARSHIP_SOLO_INTERNAL_ERROR;
        --public_state.history.outgoing_count;
        --public_state.history.incoming_count;
        memset(&public_state.history.outgoing[public_state.history.outgoing_count],
               0, sizeof public_state.history.outgoing[0]);
        memset(&public_state.history.incoming[public_state.history.incoming_count],
               0, sizeof public_state.history.incoming[0]);
        public_state.history.verdict = WARSHIP_BATTLE_CONTINUE;
    }
    *out = public_state;
    return WARSHIP_SOLO_OK;
}

WarshipSoloStatus warship_solo_take_final(WarshipSoloSession *session,
                                          WarshipBattleVerdict *out)
{
    if (!valid(session) || out == NULL) return WARSHIP_SOLO_BAD_ARGUMENT;
    if (session->phase != WARSHIP_SOLO_FINISHED)
        return WARSHIP_SOLO_NO_EVENT;
    const WarshipBattleStatus status =
        warship_battle_take_final(&session->battle, out);
    return status == WARSHIP_BATTLE_NO_EVENT ? WARSHIP_SOLO_NO_EVENT :
           status == WARSHIP_BATTLE_OK ? WARSHIP_SOLO_OK :
           WARSHIP_SOLO_INTERNAL_ERROR;
}
