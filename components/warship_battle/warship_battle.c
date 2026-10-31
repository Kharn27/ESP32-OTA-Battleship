#include "warship_battle.h"

#include <string.h>

#define BATTLE_MAGIC UINT32_C(0x42303231)

static int valid(const WarshipBattle *battle)
{
    return battle != NULL && battle->magic == BATTLE_MAGIC &&
           battle->phase <= WARSHIP_BATTLE_FINISHED;
}

static int valid_side(WarshipBattleSide side)
{
    return side == WARSHIP_BATTLE_HUMAN || side == WARSHIP_BATTLE_CAPTAIN;
}

static unsigned index_of(unsigned x, unsigned y)
{
    return y * WARSHIP_FLEET_GRID + x;
}

WarshipBattleStatus warship_battle_init(WarshipBattle *battle,
                                        const WarshipFleet *human,
                                        const WarshipFleet *captain)
{
    WarshipBattle state;
    if (battle == NULL || human == NULL || captain == NULL)
        return WARSHIP_BATTLE_BAD_ARGUMENT;
    if (warship_fleet_validate(human) != WARSHIP_FLEET_OK ||
        warship_fleet_validate(captain) != WARSHIP_FLEET_OK)
        return WARSHIP_BATTLE_BAD_FLEET;
    memset(&state, 0, sizeof state);
    state.fleets[WARSHIP_BATTLE_HUMAN] = *human;
    state.fleets[WARSHIP_BATTLE_CAPTAIN] = *captain;
    state.phase = WARSHIP_BATTLE_PREPARING;
    state.verdict = WARSHIP_BATTLE_CONTINUE;
    state.magic = BATTLE_MAGIC;
    *battle = state;
    return WARSHIP_BATTLE_OK;
}

static WarshipBattleStatus check_actor(const WarshipBattle *battle,
                                       WarshipBattleSide side)
{
    if (!valid(battle)) return WARSHIP_BATTLE_BAD_ARGUMENT;
    if (!valid_side(side)) return WARSHIP_BATTLE_BAD_SIDE;
    if (battle->phase != WARSHIP_BATTLE_PREPARING)
        return WARSHIP_BATTLE_BAD_PHASE;
    if (battle->locked[side] != 0u)
        return WARSHIP_BATTLE_ALREADY_LOCKED;
    return WARSHIP_BATTLE_OK;
}

WarshipBattleStatus warship_battle_select(WarshipBattle *battle,
                                          WarshipBattleSide side,
                                          uint8_t x, uint8_t y)
{
    WarshipBattleStatus status = check_actor(battle, side);
    if (status != WARSHIP_BATTLE_OK) return status;
    if (x >= WARSHIP_FLEET_GRID || y >= WARSHIP_FLEET_GRID)
        return WARSHIP_BATTLE_OUT_OF_BOUNDS;
    if (battle->attacked[side][index_of(x, y)] != 0u)
        return WARSHIP_BATTLE_ALREADY_ATTACKED;
    if (battle->selected[side] != 0u &&
        battle->target_x[side] == x && battle->target_y[side] == y)
        return WARSHIP_BATTLE_DUPLICATE_SELECTION;
    battle->target_x[side] = x;
    battle->target_y[side] = y;
    battle->selected[side] = 1u;
    return WARSHIP_BATTLE_OK;
}

WarshipBattleStatus warship_battle_cancel(WarshipBattle *battle,
                                          WarshipBattleSide side)
{
    WarshipBattleStatus status = check_actor(battle, side);
    if (status != WARSHIP_BATTLE_OK) return status;
    if (battle->selected[side] == 0u) return WARSHIP_BATTLE_NO_SELECTION;
    battle->selected[side] = 0u;
    return WARSHIP_BATTLE_OK;
}

WarshipBattleStatus warship_battle_fire(WarshipBattle *battle,
                                        WarshipBattleSide side)
{
    WarshipBattleStatus status = check_actor(battle, side);
    if (status != WARSHIP_BATTLE_OK) return status;
    if (battle->selected[side] == 0u) return WARSHIP_BATTLE_NO_SELECTION;
    battle->locked[side] = 1u;
    if (battle->locked[WARSHIP_BATTLE_HUMAN] != 0u &&
        battle->locked[WARSHIP_BATTLE_CAPTAIN] != 0u)
        battle->phase = WARSHIP_BATTLE_READY;
    return WARSHIP_BATTLE_OK;
}

static WarshipBattleShot attack(WarshipBattle *state, unsigned attacker)
{
    const unsigned victim = 1u - attacker;
    const unsigned x = state->target_x[attacker];
    const unsigned y = state->target_y[attacker];
    WarshipBattleShot shot = {(uint8_t)x, (uint8_t)y,
                              WARSHIP_BATTLE_MISS, WARSHIP_BATTLE_NO_SHIP};
    const WarshipFleet *fleet = &state->fleets[victim];
    for (unsigned i = 0; i < WARSHIP_FLEET_SHIPS; ++i) {
        const WarshipShip *ship = &fleet->ships[i];
        const unsigned sx = ship->x;
        const unsigned sy = ship->y;
        const unsigned length = ship->length;
        const int inside = ship->orientation == WARSHIP_ORIENT_HORIZONTAL ?
            (y == sy && x >= sx && x - sx < length) :
            (x == sx && y >= sy && y - sy < length);
        if (inside) {
            const uint8_t id = ship->id;
            ++state->hit_by_ship[victim][id];
            ++state->total_hits[victim];
            shot.impact = WARSHIP_BATTLE_HIT;
            if (state->hit_by_ship[victim][id] == length) {
                shot.impact = WARSHIP_BATTLE_SUNK;
                shot.ship_id = id;
            }
            break;
        }
    }
    state->attacked[attacker][index_of(x, y)] = 1u;
    state->history[attacker][state->counts[attacker]] = shot;
    ++state->counts[attacker];
    return shot;
}

WarshipBattleStatus warship_battle_resolve(WarshipBattle *battle,
                                           WarshipBattlePair *pair)
{
    WarshipBattle next;
    WarshipBattlePair result;
    if (!valid(battle) || pair == NULL) return WARSHIP_BATTLE_BAD_ARGUMENT;
    if (battle->phase != WARSHIP_BATTLE_READY) return WARSHIP_BATTLE_BAD_PHASE;
    for (unsigned i = 0; i < WARSHIP_BATTLE_SIDES; ++i) {
        if (battle->locked[i] == 0u || battle->selected[i] == 0u ||
            battle->target_x[i] >= WARSHIP_FLEET_GRID ||
            battle->target_y[i] >= WARSHIP_FLEET_GRID ||
            battle->counts[i] >= WARSHIP_BATTLE_CELLS ||
            battle->attacked[i][index_of(battle->target_x[i],
                                         battle->target_y[i])] != 0u)
            return WARSHIP_BATTLE_BAD_PHASE;
    }
    /* Stage all changes and both outgoing results before committing either. */
    next = *battle;
    memset(&result, 0, sizeof result);
    result.human = attack(&next, WARSHIP_BATTLE_HUMAN);
    result.captain = attack(&next, WARSHIP_BATTLE_CAPTAIN);
    result.round = next.counts[WARSHIP_BATTLE_HUMAN];
    if (next.total_hits[WARSHIP_BATTLE_CAPTAIN] == WARSHIP_FLEET_CELLS &&
        next.total_hits[WARSHIP_BATTLE_HUMAN] == WARSHIP_FLEET_CELLS)
        result.verdict = WARSHIP_BATTLE_DRAW;
    else if (next.total_hits[WARSHIP_BATTLE_CAPTAIN] == WARSHIP_FLEET_CELLS)
        result.verdict = WARSHIP_BATTLE_VICTORY;
    else if (next.total_hits[WARSHIP_BATTLE_HUMAN] == WARSHIP_FLEET_CELLS)
        result.verdict = WARSHIP_BATTLE_DEFEAT;
    else
        result.verdict = WARSHIP_BATTLE_CONTINUE;
    next.verdict = result.verdict;
    next.selected[WARSHIP_BATTLE_HUMAN] = 0u;
    next.selected[WARSHIP_BATTLE_CAPTAIN] = 0u;
    next.locked[WARSHIP_BATTLE_HUMAN] = 0u;
    next.locked[WARSHIP_BATTLE_CAPTAIN] = 0u;
    if (result.verdict == WARSHIP_BATTLE_CONTINUE)
        next.phase = WARSHIP_BATTLE_AWAIT_REVEAL;
    else {
        next.phase = WARSHIP_BATTLE_FINISHED;
        next.final_event_pending = 1u;
    }
    *battle = next;
    *pair = result;
    return WARSHIP_BATTLE_OK;
}

WarshipBattleStatus warship_battle_finish_reveal(WarshipBattle *battle)
{
    if (!valid(battle)) return WARSHIP_BATTLE_BAD_ARGUMENT;
    if (battle->phase != WARSHIP_BATTLE_AWAIT_REVEAL)
        return WARSHIP_BATTLE_BAD_PHASE;
    battle->phase = WARSHIP_BATTLE_PREPARING;
    return WARSHIP_BATTLE_OK;
}

WarshipBattleStatus warship_battle_take_final(WarshipBattle *battle,
                                              WarshipBattleVerdict *verdict)
{
    if (!valid(battle) || verdict == NULL)
        return WARSHIP_BATTLE_BAD_ARGUMENT;
    if (battle->phase != WARSHIP_BATTLE_FINISHED ||
        battle->final_event_pending == 0u)
        return WARSHIP_BATTLE_NO_EVENT;
    *verdict = (WarshipBattleVerdict)battle->verdict;
    battle->final_event_pending = 0u;
    return WARSHIP_BATTLE_OK;
}

WarshipBattleStatus warship_battle_observe(const WarshipBattle *battle,
                                           WarshipBattleSide observer,
                                           WarshipBattlePublic *out)
{
    WarshipBattlePublic public_view;
    if (!valid(battle) || out == NULL) return WARSHIP_BATTLE_BAD_ARGUMENT;
    if (!valid_side(observer)) return WARSHIP_BATTLE_BAD_SIDE;
    memset(&public_view, 0, sizeof public_view);
    const unsigned s = (unsigned)observer;
    const unsigned opponent = 1u - s;
    public_view.outgoing_count = battle->counts[s];
    public_view.incoming_count = battle->counts[opponent];
    public_view.verdict = battle->verdict;
    memcpy(public_view.outgoing, battle->history[s],
           sizeof public_view.outgoing);
    memcpy(public_view.incoming, battle->history[opponent],
           sizeof public_view.incoming);
    *out = public_view;
    return WARSHIP_BATTLE_OK;
}

WarshipBattlePhase warship_battle_phase(const WarshipBattle *battle)
{
    if (!valid(battle)) return WARSHIP_BATTLE_FINISHED; /* fail closed */
    return (WarshipBattlePhase)battle->phase;
}
