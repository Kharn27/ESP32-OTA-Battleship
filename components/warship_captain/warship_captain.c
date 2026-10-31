#include "warship_captain.h"

#include <string.h>

#define BOARD_CELLS WARSHIP_BATTLE_CELLS
#define GRID WARSHIP_FLEET_GRID

static unsigned cell(unsigned x, unsigned y)
{
    return y * GRID + x;
}

/* Count/coordinate/impact/type/duplicate checks for both D02 histories.
 * Ignore unused trailing entries, as specified by the D02 count contract.
 */
static int valid_history(const WarshipBattleShot *shots, unsigned count)
{
    uint8_t seen[BOARD_CELLS] = {0};
    unsigned sunk_ids = 0u;
    for (unsigned i = 0u; i < count; ++i) {
        const WarshipBattleShot *shot = &shots[i];
        if (shot->x >= GRID || shot->y >= GRID ||
            shot->impact > WARSHIP_BATTLE_SUNK)
            return 0;
        if (shot->impact == WARSHIP_BATTLE_SUNK) {
            if (shot->ship_id >= WARSHIP_FLEET_SHIPS ||
                (sunk_ids & (1u << shot->ship_id)) != 0u)
                return 0;
            sunk_ids |= 1u << shot->ship_id;
        } else if (shot->ship_id != WARSHIP_BATTLE_NO_SHIP) {
            return 0;
        }
        const unsigned where = cell(shot->x, shot->y);
        if (seen[where] != 0u) return 0;
        seen[where] = 1u;
    }
    return 1;
}

static int valid_view(const WarshipBattlePublic *view)
{
    if (view->outgoing_count > BOARD_CELLS ||
        view->incoming_count > BOARD_CELLS ||
        view->outgoing_count != view->incoming_count ||
        view->verdict > WARSHIP_BATTLE_DRAW)
        return 0;
    return valid_history(view->outgoing, view->outgoing_count) &&
           valid_history(view->incoming, view->incoming_count);
}

/* Returns 0..2 orthogonal endpoints for a hypothesized contiguous pair.
 * Pair is only a HYPOTHESIS: D01 allows touching ships. No owner labels.
 */
static unsigned endpoints(unsigned a, unsigned b, const uint8_t used[BOARD_CELLS],
                          unsigned result[2])
{
    unsigned n = 0u;
    if (b == a + 1u && a / GRID == b / GRID) {
        if (a % GRID > 0u && !used[a - 1u]) result[n++] = a - 1u;
        if (b % GRID + 1u < GRID && !used[b + 1u]) result[n++] = b + 1u;
    } else if (b == a + GRID) {
        if (a >= GRID && !used[a - GRID]) result[n++] = a - GRID;
        if (b + GRID < BOARD_CELLS && !used[b + GRID])
            result[n++] = b + GRID;
    }
    return n;
}

static unsigned neighbors(unsigned pos, const uint8_t used[BOARD_CELLS],
                          unsigned result[4])
{
    unsigned n = 0u;
    const unsigned x = pos % GRID;
    if (x > 0u && !used[pos - 1u]) result[n++] = pos - 1u;
    if (x + 1u < GRID && !used[pos + 1u]) result[n++] = pos + 1u;
    if (pos >= GRID && !used[pos - GRID]) result[n++] = pos - GRID;
    if (pos + GRID < BOARD_CELLS && !used[pos + GRID])
        result[n++] = pos + GRID;
    return n;
}

/* First pursue an extendable aligned HIT pair, newest evidence first;
 * then an unknown orthogonal neighbor of the newest remaining HIT.
 * SUNK reports reveal only type, never the membership of older HITs.
 * More nuanced post-sink / multiple-track reasoning is outside D03.
 */
static int pursue(const WarshipBattlePublic *view,
                  const uint8_t used[BOARD_CELLS],
                  const uint8_t hit[BOARD_CELLS], uint32_t sample,
                  unsigned *chosen)
{
    for (unsigned i = view->outgoing_count; i > 0u; --i) {
        const WarshipBattleShot *shot = &view->outgoing[i - 1u];
        if (shot->impact != WARSHIP_BATTLE_HIT) continue;
        const unsigned pos = cell(shot->x, shot->y);
        const unsigned x = pos % GRID;
        unsigned c[2];
        unsigned n;
        if (x + 1u < GRID && hit[pos + 1u]) {
            n = endpoints(pos, pos + 1u, used, c);
            if (n != 0u) { *chosen = c[sample % n]; return 1; }
        }
        if (x > 0u && hit[pos - 1u]) {
            n = endpoints(pos - 1u, pos, used, c);
            if (n != 0u) { *chosen = c[sample % n]; return 1; }
        }
        if (pos + GRID < BOARD_CELLS && hit[pos + GRID]) {
            n = endpoints(pos, pos + GRID, used, c);
            if (n != 0u) { *chosen = c[sample % n]; return 1; }
        }
        if (pos >= GRID && hit[pos - GRID]) {
            n = endpoints(pos - GRID, pos, used, c);
            if (n != 0u) { *chosen = c[sample % n]; return 1; }
        }
    }
    for (unsigned i = view->outgoing_count; i > 0u; --i) {
        const WarshipBattleShot *shot = &view->outgoing[i - 1u];
        if (shot->impact != WARSHIP_BATTLE_HIT) continue;
        const unsigned pos = cell(shot->x, shot->y);
        unsigned c[4];
        const unsigned n = neighbors(pos, used, c);
        if (n != 0u) { *chosen = c[sample % n]; return 1; }
    }
    return 0;
}

/* Search without a useful HIT: distribute exploration over four 5x5 zones,
 * with a variable checkerboard phase and random tie-breaking. Not a
 * probability enumeration or a fixed sequence common to every game.
 */
static unsigned explore(const uint8_t used[BOARD_CELLS], uint32_t sample)
{
    unsigned activity[4] = {0u, 0u, 0u, 0u};
    unsigned parity = sample & 1u;
    for (unsigned i = 0u; i < BOARD_CELLS; ++i) {
        if (used[i]) {
            unsigned zone = (i % GRID >= GRID / 2u ? 1u : 0u) +
                            (i / GRID >= GRID / 2u ? 2u : 0u);
            ++activity[zone];
        }
    }
    /* If a checkerboard color is exhausted, use the other. */
    unsigned viable = 0u;
    for (unsigned i = 0u; i < BOARD_CELLS; ++i)
        if (!used[i] && ((i % GRID + i / GRID) & 1u) == parity)
            ++viable;
    if (viable == 0u) parity ^= 1u;

    unsigned least = BOARD_CELLS + 1u;
    unsigned count = 0u;
    for (unsigned i = 0u; i < BOARD_CELLS; ++i) {
        if (used[i] || ((i % GRID + i / GRID) & 1u) != parity)
            continue;
        const unsigned zone = (i % GRID >= GRID / 2u ? 1u : 0u) +
                              (i / GRID >= GRID / 2u ? 2u : 0u);
        if (activity[zone] < least) { least = activity[zone]; count = 0u; }
        if (activity[zone] == least) ++count;
    }
    /* count > 0: at least one free square was checked by caller. */
    unsigned offset = (sample >> 1u) % count;
    for (unsigned i = 0u; i < BOARD_CELLS; ++i) {
        if (used[i] || ((i % GRID + i / GRID) & 1u) != parity)
            continue;
        const unsigned zone = (i % GRID >= GRID / 2u ? 1u : 0u) +
                              (i / GRID >= GRID / 2u ? 2u : 0u);
        if (activity[zone] == least && offset-- == 0u) return i;
    }
    return 0u; /* Unreachable for a validated, non-full board. */
}

WarshipCaptainStatus warship_captain_choose(const WarshipBattlePublic *view,
                                            WarshipCaptainRandom random_u32,
                                            void *random_context,
                                            WarshipCaptainTarget *out)
{
    uint8_t used[BOARD_CELLS] = {0};
    uint8_t hit[BOARD_CELLS] = {0};
    if (view == NULL || random_u32 == NULL || out == NULL)
        return WARSHIP_CAPTAIN_BAD_ARGUMENT;
    if (!valid_view(view)) return WARSHIP_CAPTAIN_BAD_OBSERVATION;
    if (view->verdict != WARSHIP_BATTLE_CONTINUE ||
        view->outgoing_count == BOARD_CELLS)
        return WARSHIP_CAPTAIN_NO_TARGET;

    for (unsigned i = 0u; i < view->outgoing_count; ++i) {
        const WarshipBattleShot *shot = &view->outgoing[i];
        const unsigned pos = cell(shot->x, shot->y);
        used[pos] = 1u;
        if (shot->impact == WARSHIP_BATTLE_HIT) hit[pos] = 1u;
    }
    uint32_t sample = 0u;
    if (random_u32(random_context, &sample) != 0)
        return WARSHIP_CAPTAIN_RANDOM_FAILURE;
    unsigned chosen = 0u;
    if (!pursue(view, used, hit, sample, &chosen))
        chosen = explore(used, sample);
    WarshipCaptainTarget target = {(uint8_t)(chosen % GRID),
                                   (uint8_t)(chosen / GRID)};
    *out = target;
    return WARSHIP_CAPTAIN_OK;
}
