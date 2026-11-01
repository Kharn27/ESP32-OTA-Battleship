#include "warship_captain.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct { uint32_t value; unsigned calls; int fail; } TestRng;
static int fixed_rng(void *context, uint32_t *value)
{
    TestRng *rng = (TestRng *)context;
    ++rng->calls;
    if (rng->fail) return -1;
    *value = rng->value;
    return 0;
}
static void add(WarshipBattlePublic *view, unsigned x, unsigned y,
                unsigned impact, unsigned ship_id)
{
    assert(view->outgoing_count < WARSHIP_BATTLE_CELLS);
    const WarshipBattleShot shot = {(uint8_t)x, (uint8_t)y,
                                    (uint8_t)impact, (uint8_t)ship_id};
    view->outgoing[view->outgoing_count++] = shot;
    view->incoming[view->incoming_count++] = shot;
}
static WarshipCaptainTarget pick(const WarshipBattlePublic *view, uint32_t value)
{
    TestRng rng = {value, 0u, 0};
    WarshipCaptainTarget target = {255u, 255u};
    assert(warship_captain_choose(view, fixed_rng, &rng, &target) ==
           WARSHIP_CAPTAIN_OK);
    assert(rng.calls == 1u && target.x < 10u && target.y < 10u);
    for (unsigned i = 0u; i < view->outgoing_count; ++i)
        assert(view->outgoing[i].x != target.x ||
               view->outgoing[i].y != target.y);
    return target;
}
/* Synthetic public history: all nonfree squares are MISS except designated
 * SUNK impact coordinates. Deliberately does not create a secret fleet.
 */
static void layout(WarshipBattlePublic *view, const uint8_t free_cells[100],
                   unsigned sunk_mask, int transpose)
{
    memset(view, 0, sizeof *view);
    const unsigned sx[5] = {4u, 0u, 1u, 2u, 3u};
    const unsigned sy[5] = {6u, 9u, 9u, 9u, 9u};
    for (unsigned y = 0u; y < 10u; ++y)
        for (unsigned x = 0u; x < 10u; ++x) {
            if (free_cells[y * 10u + x]) continue;
            unsigned impact = WARSHIP_BATTLE_MISS;
            unsigned id = WARSHIP_BATTLE_NO_SHIP;
            for (unsigned k = 0u; k < 5u; ++k)
                if ((sunk_mask & (1u << k)) && x == sx[k] && y == sy[k]) {
                    impact = WARSHIP_BATTLE_SUNK;
                    id = k;
                    break;
                }
            add(view, transpose ? y : x, transpose ? x : y, impact, id);
        }
}
static void two_and_five(uint8_t free_cells[100])
{
    memset(free_cells, 0, 100u);
    free_cells[2u * 10u + 2u] = 1u;
    free_cells[2u * 10u + 3u] = 1u; /* isolated two-square pocket */
    for (unsigned x = 5u; x <= 9u; ++x)
        free_cells[6u * 10u + x] = 1u; /* five-square corridor */
    /* Isolated decoys make the impossible upper-left quadrant LESS used
     * than the credible lower-right one. D03 alone would aim there. */
    free_cells[0u] = 1u;
    free_cells[4u] = 1u;
    free_cells[40u] = 1u;
    free_cells[44u] = 1u;
}
static void three_and_five(uint8_t free_cells[100])
{
    memset(free_cells, 0, 100u);
    for (unsigned x = 1u; x <= 3u; ++x)
        free_cells[2u * 10u + x] = 1u;
    for (unsigned x = 5u; x <= 9u; ++x)
        free_cells[6u * 10u + x] = 1u;
    /* Equal zone activity: sample zero selects the size-3 pocket iff
     * that distinct size-3 ship has NOT been announced sunk. */
    free_cells[0u] = 1u;
    free_cells[44u] = 1u;
}
static void test_two_square_exclusion(void)
{
    uint8_t free_cells[100];
    two_and_five(free_cells);
    for (int transpose = 0; transpose < 2; ++transpose) {
        WarshipBattlePublic view;
        layout(&view, free_cells, 1u << WARSHIP_SHIP_PATROL, transpose);
        assert(view.outgoing_count == 89u);
        for (uint32_t sample = 0u; sample < 64u; ++sample) {
            const WarshipCaptainTarget target = pick(&view, sample);
            const unsigned x = transpose ? target.y : target.x;
            const unsigned y = transpose ? target.x : target.y;
            assert(y == 6u && x >= 5u && x <= 9u);
        }
    }
}
static void test_distinct_ship_types(void)
{
    assert(warship_fleet_ship_length(WARSHIP_SHIP_PATROL) == 2u);
    assert(warship_fleet_ship_length(WARSHIP_SHIP_SUBMARINE) == 3u);
    assert(warship_fleet_ship_length(WARSHIP_SHIP_DESTROYER) == 3u);
    assert(warship_fleet_ship_length(WARSHIP_SHIP_CRUISER) == 4u);
    assert(warship_fleet_ship_length(WARSHIP_SHIP_CARRIER) == 5u);

    uint8_t free_cells[100];
    WarshipBattlePublic view;
    three_and_five(free_cells);
    /* Sinking only one of the two length-3 IDs must leave the pocket
     * credible. With this public map and RNG=0 the target is (2,2).
     * Without filtering, the isolated decoy (0,0) wins instead. */
    layout(&view, free_cells, (1u << WARSHIP_SHIP_PATROL) |
                              (1u << WARSHIP_SHIP_SUBMARINE), 0);
    WarshipCaptainTarget t = pick(&view, 0u);
    assert(t.x == 2u && t.y == 2u);
    /* Once BOTH 3-cell IDs are sunk, the 3-cell pocket is excluded. */
    layout(&view, free_cells, (1u << WARSHIP_SHIP_PATROL) |
                              (1u << WARSHIP_SHIP_SUBMARINE) |
                              (1u << WARSHIP_SHIP_DESTROYER), 0);
    for (uint32_t sample = 0u; sample < 64u; ++sample) {
        t = pick(&view, sample);
        assert(t.y == 6u && t.x >= 5u && t.x <= 9u);
    }

    /* The patrol remains afloat even when all larger ships are sunk.
     * Two-square pocket must remain selectable, isolated singles excluded. */
    two_and_five(free_cells);
    layout(&view, free_cells, 0x1eu, 0);
    t = pick(&view, 0u);
    assert(t.x == 2u && t.y == 2u);
    for (uint32_t sample = 0u; sample < 64u; ++sample) {
        t = pick(&view, sample);
        assert((t.y == 2u && (t.x == 2u || t.x == 3u)) ||
               (t.y == 6u && t.x >= 5u && t.x <= 9u));
    }

    /* All 2/3/3/4 publicly SUNK: only length five may guide the filter. */
    three_and_five(free_cells);
    layout(&view, free_cells, 0x0fu, 0);
    for (uint32_t sample = 0u; sample < 64u; ++sample) {
        t = pick(&view, sample);
        assert(t.y == 6u && t.x >= 5u && t.x <= 9u);
    }
}
static void test_public_hit_and_touching(void)
{
    WarshipBattlePublic view = {0};
    add(&view, 4u, 4u, WARSHIP_BATTLE_HIT, WARSHIP_BATTLE_NO_SHIP);
    add(&view, 5u, 5u, WARSHIP_BATTLE_SUNK, WARSHIP_SHIP_PATROL);
    /* Previous HIT adjacent diagonally to known SUNK stays a live clue;
     * neither that HIT nor its neighboring unknowns become water. */
    for (uint32_t sample = 0u; sample < 16u; ++sample) {
        const WarshipCaptainTarget t = pick(&view, sample);
        assert((t.x == 3u && t.y == 4u) ||
               (t.x == 5u && t.y == 4u) ||
               (t.x == 4u && t.y == 3u) ||
               (t.x == 4u && t.y == 5u));
    }
    /* No hidden fleet information; different incoming histories do not
     * change the target when the outgoing observation/RNG are equal. */
    WarshipBattlePublic other = view;
    other.incoming[0].x = 9u;
    other.incoming[0].y = 0u;
    other.incoming[1].x = 8u;
    other.incoming[1].y = 0u;
    for (uint32_t s = 0u; s < 16u; ++s) {
        WarshipCaptainTarget a = pick(&view, s);
        WarshipCaptainTarget b = pick(&other, s);
        assert(memcmp(&a, &b, sizeof a) == 0);
    }
}
static void test_fallback_and_rng_errors(void)
{
    uint8_t free_cells[100] = {0};
    free_cells[42u] = 1u;
    WarshipBattlePublic view;
    layout(&view, free_cells, 1u << WARSHIP_SHIP_PATROL, 0);
    const WarshipCaptainTarget last = pick(&view, UINT32_MAX);
    assert(last.x == 2u && last.y == 4u); /* no length-3 run */

    /* Artificial state with every ship announced sunk: no divide-by-zero. */
    layout(&view, free_cells, 0x1fu, 0);
    const WarshipCaptainTarget exhausted = pick(&view, 0u);
    assert(exhausted.x == 2u && exhausted.y == 4u);

    add(&view, 2u, 4u, WARSHIP_BATTLE_MISS, WARSHIP_BATTLE_NO_SHIP);
    TestRng rng = {42u, 0u, 0};
    WarshipCaptainTarget out = {77u, 88u};
    assert(warship_captain_choose(&view, fixed_rng, &rng, &out) ==
           WARSHIP_CAPTAIN_NO_TARGET);
    assert(rng.calls == 0u && out.x == 77u && out.y == 88u);

    view.outgoing[0].ship_id = WARSHIP_SHIP_PATROL;
    assert(warship_captain_choose(&view, fixed_rng, &rng, &out) ==
           WARSHIP_CAPTAIN_BAD_OBSERVATION);
    assert(rng.calls == 0u && out.x == 77u && out.y == 88u);

    WarshipBattlePublic empty = {0};
    rng.fail = 1;
    assert(warship_captain_choose(&empty, fixed_rng, &rng, &out) ==
           WARSHIP_CAPTAIN_RANDOM_FAILURE);
    assert(rng.calls == 1u && out.x == 77u && out.y == 88u);
}
static void test_flexible_exploration(void)
{
    uint8_t free_cells[100];
    two_and_five(free_cells);
    WarshipBattlePublic view;
    layout(&view, free_cells, 1u << WARSHIP_SHIP_PATROL, 0);
    uint8_t chosen[100] = {0};
    unsigned distinct = 0u;
    for (uint32_t sample = 0u; sample < 128u; ++sample) {
        const WarshipCaptainTarget t = pick(&view, sample);
        const unsigned pos = (unsigned)t.y * 10u + t.x;
        if (!chosen[pos]) { chosen[pos] = 1u; ++distinct; }
    }
    assert(distinct >= 2u); /* non-unique, non-perfect choice */
}
int main(void)
{
    test_two_square_exclusion();
    test_distinct_ship_types();
    test_public_hit_and_touching();
    test_fallback_and_rng_errors();
    test_flexible_exploration();
    printf("D05 PASS sizeof(public)=%zu sizeof(target)=%zu\n",
           sizeof(WarshipBattlePublic), sizeof(WarshipCaptainTarget));
    return 0;
}
