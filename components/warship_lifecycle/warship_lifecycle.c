#include "warship_lifecycle.h"

#include <string.h>

#define LIFECYCLE_MAGIC UINT32_C(0x4c303731)

static int valid(const WarshipLifecycle *game)
{
    return game != NULL && game->magic == LIFECYCLE_MAGIC &&
           game->phase <= WARSHIP_LIFECYCLE_RESULT;
}

static WarshipLifecycleStatus from_solo(WarshipSoloStatus s)
{
    switch (s) {
    case WARSHIP_SOLO_OK: return WARSHIP_LIFECYCLE_OK;
    case WARSHIP_SOLO_BAD_ARGUMENT: return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    case WARSHIP_SOLO_BAD_PHASE: return WARSHIP_LIFECYCLE_BAD_PHASE;
    case WARSHIP_SOLO_NO_SELECTION: return WARSHIP_LIFECYCLE_NO_SELECTION;
    case WARSHIP_SOLO_OUT_OF_BOUNDS: return WARSHIP_LIFECYCLE_OUT_OF_BOUNDS;
    case WARSHIP_SOLO_ALREADY_ATTACKED:
        return WARSHIP_LIFECYCLE_ALREADY_ATTACKED;
    case WARSHIP_SOLO_DUPLICATE_SELECTION:
        return WARSHIP_LIFECYCLE_DUPLICATE_SELECTION;
    case WARSHIP_SOLO_ALREADY_FIRED:
        return WARSHIP_LIFECYCLE_ALREADY_FIRED;
    case WARSHIP_SOLO_RANDOM_FAILURE:
        return WARSHIP_LIFECYCLE_RANDOM_FAILURE;
    default: return WARSHIP_LIFECYCLE_INTERNAL_ERROR;
    }
}

static void go_home(WarshipLifecycle *game)
{
    /* Do not retain sensitive placements after returning home. */
    WarshipLifecycle next;
    memset(&next, 0, sizeof next);
    next.magic = LIFECYCLE_MAGIC;
    next.phase = WARSHIP_LIFECYCLE_HOME;
    *game = next;
}

static int ready_fleets(const WarshipLifecycle *game)
{
    return warship_fleet_validate(&game->human) == WARSHIP_FLEET_OK &&
           warship_fleet_validate(&game->captain) == WARSHIP_FLEET_OK;
}

WarshipLifecycleStatus warship_lifecycle_init(WarshipLifecycle *game)
{
    if (game == NULL) return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    go_home(game);
    return WARSHIP_LIFECYCLE_OK;
}

WarshipLifecycleStatus warship_lifecycle_start(WarshipLifecycle *game,
                                               WarshipFleetRandom human_rng,
                                               void *human_context,
                                               WarshipFleetRandom captain_rng,
                                               void *captain_context)
{
    if (!valid(game) || human_rng == NULL || captain_rng == NULL)
        return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    if (game->phase != WARSHIP_LIFECYCLE_HOME)
        return WARSHIP_LIFECYCLE_BAD_PHASE;
    WarshipFleet human, captain;
    if (warship_fleet_generate(&human, human_rng, human_context) !=
        WARSHIP_FLEET_OK) return WARSHIP_LIFECYCLE_RANDOM_FAILURE;
    if (warship_fleet_generate(&captain, captain_rng, captain_context) !=
        WARSHIP_FLEET_OK) return WARSHIP_LIFECYCLE_RANDOM_FAILURE;
    game->human = human;
    game->captain = captain;
    game->phase = WARSHIP_LIFECYCLE_DEPLOY;
    return WARSHIP_LIFECYCLE_OK;
}

WarshipLifecycleStatus warship_lifecycle_auto(WarshipLifecycle *game,
                                              WarshipFleetRandom human_rng,
                                              void *human_context)
{
    if (!valid(game) || human_rng == NULL)
        return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    if (game->phase != WARSHIP_LIFECYCLE_DEPLOY)
        return WARSHIP_LIFECYCLE_BAD_PHASE;
    WarshipFleet replacement;
    if (warship_fleet_generate(&replacement, human_rng, human_context) !=
        WARSHIP_FLEET_OK) return WARSHIP_LIFECYCLE_RANDOM_FAILURE;
    game->human = replacement;
    return WARSHIP_LIFECYCLE_OK;
}

WarshipLifecycleStatus warship_lifecycle_ready(WarshipLifecycle *game)
{
    if (!valid(game)) return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    if (game->phase != WARSHIP_LIFECYCLE_DEPLOY)
        return WARSHIP_LIFECYCLE_BAD_PHASE;
    if (!ready_fleets(game)) return WARSHIP_LIFECYCLE_INTERNAL_ERROR;
    WarshipSoloSession session;
    const WarshipSoloStatus st =
        warship_solo_init(&session, &game->human, &game->captain);
    if (st != WARSHIP_SOLO_OK) return from_solo(st);
    game->solo = session;
    game->phase = WARSHIP_LIFECYCLE_COMBAT;
    return WARSHIP_LIFECYCLE_OK;
}

WarshipLifecycleStatus warship_lifecycle_leave_deploy(WarshipLifecycle *game)
{
    if (!valid(game)) return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    if (game->phase != WARSHIP_LIFECYCLE_DEPLOY)
        return WARSHIP_LIFECYCLE_BAD_PHASE;
    go_home(game);
    return WARSHIP_LIFECYCLE_OK;
}

WarshipLifecycleStatus warship_lifecycle_select(WarshipLifecycle *game,
                                                uint8_t x, uint8_t y)
{
    if (!valid(game)) return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    if (game->phase != WARSHIP_LIFECYCLE_COMBAT)
        return WARSHIP_LIFECYCLE_BAD_PHASE;
    return from_solo(warship_solo_select(&game->solo, x, y));
}

WarshipLifecycleStatus warship_lifecycle_cancel(WarshipLifecycle *game)
{
    if (!valid(game)) return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    if (game->phase != WARSHIP_LIFECYCLE_COMBAT)
        return WARSHIP_LIFECYCLE_BAD_PHASE;
    return from_solo(warship_solo_cancel(&game->solo));
}

WarshipLifecycleStatus warship_lifecycle_fire(WarshipLifecycle *game,
                                              WarshipCaptainRandom rng,
                                              void *rng_context,
                                              WarshipBattlePair *pair)
{
    if (!valid(game) || rng == NULL || pair == NULL)
        return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    if (game->phase != WARSHIP_LIFECYCLE_COMBAT)
        return WARSHIP_LIFECYCLE_BAD_PHASE;
    return from_solo(warship_solo_fire(&game->solo, rng,
                                       rng_context, pair));
}

WarshipLifecycleStatus warship_lifecycle_ack_reveal(
    WarshipLifecycle *game, WarshipLifecycleEvent *event)
{
    if (!valid(game) || event == NULL)
        return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    if (game->phase != WARSHIP_LIFECYCLE_COMBAT ||
        game->solo.phase != WARSHIP_SOLO_AWAIT_REVEAL)
        return WARSHIP_LIFECYCLE_BAD_PHASE;

    /* Finish the pair and consume D06's terminal event on a local session
     * copy: failures cannot partially commit a result/event transition.
     */
    WarshipSoloSession staged = game->solo;
    if (warship_solo_ack_reveal(&staged) != WARSHIP_SOLO_OK)
        return WARSHIP_LIFECYCLE_INTERNAL_ERROR;
    WarshipLifecycleEvent result = WARSHIP_LIFECYCLE_EVENT_NONE;
    uint8_t verdict = WARSHIP_BATTLE_CONTINUE;
    if (staged.phase == WARSHIP_SOLO_FINISHED) {
        WarshipBattleVerdict final;
        if (warship_solo_take_final(&staged, &final) != WARSHIP_SOLO_OK)
            return WARSHIP_LIFECYCLE_INTERNAL_ERROR;
        verdict = (uint8_t)final;
        switch (final) {
        case WARSHIP_BATTLE_VICTORY:
            result = WARSHIP_LIFECYCLE_EVENT_VICTORY; break;
        case WARSHIP_BATTLE_DEFEAT:
            result = WARSHIP_LIFECYCLE_EVENT_DEFEAT; break;
        case WARSHIP_BATTLE_DRAW:
            result = WARSHIP_LIFECYCLE_EVENT_DRAW; break;
        default: return WARSHIP_LIFECYCLE_INTERNAL_ERROR;
        }
    }
    game->solo = staged;
    game->verdict = verdict;
    if (result != WARSHIP_LIFECYCLE_EVENT_NONE)
        game->phase = WARSHIP_LIFECYCLE_RESULT;
    *event = result;
    return WARSHIP_LIFECYCLE_OK;
}

WarshipLifecycleStatus warship_lifecycle_request_abandon(
    WarshipLifecycle *game)
{
    if (!valid(game)) return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    if (game->phase != WARSHIP_LIFECYCLE_COMBAT ||
        game->solo.phase != WARSHIP_SOLO_SELECTING)
        return WARSHIP_LIFECYCLE_BAD_PHASE;
    game->phase = WARSHIP_LIFECYCLE_CONFIRM_ABANDON;
    return WARSHIP_LIFECYCLE_OK;
}

WarshipLifecycleStatus warship_lifecycle_decide_abandon(
    WarshipLifecycle *game, uint8_t abandon, WarshipLifecycleEvent *event)
{
    if (!valid(game) || event == NULL || abandon > 1u)
        return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    if (game->phase != WARSHIP_LIFECYCLE_CONFIRM_ABANDON)
        return WARSHIP_LIFECYCLE_BAD_PHASE;
    if (abandon != 0u) {
        go_home(game);
        *event = WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT;
    } else {
        game->phase = WARSHIP_LIFECYCLE_COMBAT;
        *event = WARSHIP_LIFECYCLE_EVENT_NONE;
    }
    return WARSHIP_LIFECYCLE_OK;
}

WarshipLifecycleStatus warship_lifecycle_replay(
    WarshipLifecycle *game, WarshipFleetRandom captain_rng,
    void *captain_context)
{
    if (!valid(game) || captain_rng == NULL)
        return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    if (game->phase != WARSHIP_LIFECYCLE_RESULT)
        return WARSHIP_LIFECYCLE_BAD_PHASE;
    /* Keep human deployment, but independently regenerate CPU placement,
     * even if its random outcome happens to coincide with the old one. */
    WarshipFleet captain;
    if (warship_fleet_generate(&captain, captain_rng, captain_context) !=
        WARSHIP_FLEET_OK) return WARSHIP_LIFECYCLE_RANDOM_FAILURE;
    game->captain = captain;
    memset(&game->solo, 0, sizeof game->solo);
    game->verdict = WARSHIP_BATTLE_CONTINUE;
    game->reveal_requested = 0u;
    game->reveal_visible = 0u;
    game->phase = WARSHIP_LIFECYCLE_DEPLOY;
    return WARSHIP_LIFECYCLE_OK;
}

WarshipLifecycleStatus warship_lifecycle_leave_result(
    WarshipLifecycle *game)
{
    if (!valid(game)) return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    if (game->phase != WARSHIP_LIFECYCLE_RESULT)
        return WARSHIP_LIFECYCLE_BAD_PHASE;
    go_home(game);
    return WARSHIP_LIFECYCLE_OK;
}

WarshipLifecycleStatus warship_lifecycle_observe(
    const WarshipLifecycle *game, WarshipLifecyclePublic *out)
{
    if (!valid(game) || out == NULL)
        return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    WarshipLifecyclePublic view;
    memset(&view, 0, sizeof view);
    view.phase = game->phase;
    if (game->phase != WARSHIP_LIFECYCLE_HOME) {
        view.has_human_fleet = 1u;
        view.human = game->human;
    }
    if (game->phase == WARSHIP_LIFECYCLE_COMBAT ||
        game->phase == WARSHIP_LIFECYCLE_CONFIRM_ABANDON ||
        game->phase == WARSHIP_LIFECYCLE_RESULT) {
        if (warship_solo_observe(&game->solo, &view.battle) !=
            WARSHIP_SOLO_OK) return WARSHIP_LIFECYCLE_INTERNAL_ERROR;
        view.has_battle = 1u;
    }
    if (game->phase == WARSHIP_LIFECYCLE_RESULT &&
        game->verdict == WARSHIP_BATTLE_DEFEAT) {
        view.can_reveal_captain = 1u;
        view.reveal_requested = game->reveal_requested;
        view.reveal_visible = game->reveal_visible;
    }
    *out = view;
    return WARSHIP_LIFECYCLE_OK;
}

WarshipLifecycleStatus warship_lifecycle_set_captain_visible(
    WarshipLifecycle *game, uint8_t visible)
{
    if (!valid(game) || visible > 1u)
        return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    if (game->phase != WARSHIP_LIFECYCLE_RESULT ||
        game->verdict != WARSHIP_BATTLE_DEFEAT)
        return WARSHIP_LIFECYCLE_REVEAL_DENIED;
    if (visible == 0u && game->reveal_requested == 0u)
        return WARSHIP_LIFECYCLE_REVEAL_DENIED;
    if (visible != 0u) game->reveal_requested = 1u;
    game->reveal_visible = visible;
    return WARSHIP_LIFECYCLE_OK;
}

WarshipLifecycleStatus warship_lifecycle_copy_revealed_captain(
    const WarshipLifecycle *game, WarshipFleet *out)
{
    if (!valid(game) || out == NULL)
        return WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    if (game->phase != WARSHIP_LIFECYCLE_RESULT ||
        game->verdict != WARSHIP_BATTLE_DEFEAT ||
        game->reveal_requested == 0u || game->reveal_visible == 0u)
        return WARSHIP_LIFECYCLE_REVEAL_DENIED;
    *out = game->captain;
    return WARSHIP_LIFECYCLE_OK;
}
