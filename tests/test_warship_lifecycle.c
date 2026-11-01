#include "warship_lifecycle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #x); \
    exit(EXIT_FAILURE); \
} } while (0)
#define EQ(a,b) CHECK((a) == (b))
#define OK(x) EQ((x), WARSHIP_LIFECYCLE_OK)

typedef struct {
    uint32_t value[WARSHIP_FLEET_SHIPS];
    unsigned calls;
    unsigned fail_at;
} FleetRng;

static int fleet_random(void *context, uint32_t *out)
{
    FleetRng *rng = (FleetRng *)context;
    ++rng->calls;
    if (rng->calls == rng->fail_at) return 1;
    *out = rng->value[(rng->calls - 1u) % WARSHIP_FLEET_SHIPS];
    return 0;
}

static FleetRng left_rng(void)
{
    FleetRng rng = {{0u,20u,40u,60u,80u}, 0u, 0u};
    return rng;
}
static FleetRng right_rng(void)
{
    FleetRng rng = {{10u,30u,50u,70u,90u}, 0u, 0u};
    return rng;
}
static FleetRng other_right_rng(void)
{
    FleetRng rng = {{12u,32u,52u,72u,92u}, 0u, 0u};
    return rng;
}
typedef struct {
    unsigned calls;
    int failure;
    uint32_t value;
} CpuRng;
static int cpu_random(void *context, uint32_t *out)
{
    CpuRng *rng = (CpuRng *)context;
    ++rng->calls;
    if (rng->failure) return 1;
    *out = rng->value;
    return 0;
}
static WarshipLifecycle initial(void)
{
    WarshipLifecycle g;
    memset(&g, 0x3a, sizeof g);
    OK(warship_lifecycle_init(&g));
    return g;
}
static void same(const WarshipLifecycle *a, const WarshipLifecycle *b)
{
    CHECK(memcmp(a, b, sizeof *a) == 0);
}
static void get(const WarshipLifecycle *g, WarshipLifecyclePublic *out)
{
    memset(out, 0x33, sizeof *out);
    OK(warship_lifecycle_observe(g, out));
}
static void deployed(WarshipLifecycle *g)
{
    FleetRng h = left_rng(), c = right_rng();
    OK(warship_lifecycle_start(g, fleet_random, &h, fleet_random, &c));
    EQ(h.calls, 5u);
    EQ(c.calls, 5u);
    EQ(warship_fleet_validate(&g->human), WARSHIP_FLEET_OK);
    EQ(warship_fleet_validate(&g->captain), WARSHIP_FLEET_OK);
}
static void ready(WarshipLifecycle *g)
{
    deployed(g);
    OK(warship_lifecycle_ready(g));
}

static void bad_arguments(void)
{
    EQ(warship_lifecycle_init(NULL), WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    WarshipLifecycle g;
    memset(&g, 0xb7, sizeof g);
    WarshipLifecycle saved = g;
    FleetRng h = left_rng(), c = right_rng();
    EQ(warship_lifecycle_start(&g, fleet_random, &h, fleet_random, &c),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    same(&g, &saved);
    EQ(warship_lifecycle_auto(&g, fleet_random, &h),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    same(&g, &saved);
    EQ(warship_lifecycle_ready(&g), WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    EQ(warship_lifecycle_leave_deploy(&g), WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    EQ(warship_lifecycle_select(&g, 0u, 0u),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    EQ(warship_lifecycle_cancel(&g), WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    CpuRng cpu = {0u, 0, 0u};
    WarshipBattlePair pair;
    memset(&pair, 0x5d, sizeof pair);
    WarshipBattlePair pair_old = pair;
    EQ(warship_lifecycle_fire(&g, cpu_random, &cpu, &pair),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    CHECK(memcmp(&pair, &pair_old, sizeof pair) == 0);
    WarshipLifecycleEvent event = WARSHIP_LIFECYCLE_EVENT_DRAW;
    EQ(warship_lifecycle_ack_reveal(&g, &event),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    EQ(warship_lifecycle_request_abandon(&g),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    EQ(warship_lifecycle_decide_abandon(&g, 1u, &event),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    EQ(warship_lifecycle_replay(&g, fleet_random, &c),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    EQ(warship_lifecycle_leave_result(&g),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    WarshipLifecyclePublic pub;
    memset(&pub, 0xa9, sizeof pub);
    WarshipLifecyclePublic oldpub = pub;
    EQ(warship_lifecycle_observe(&g, &pub),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    CHECK(memcmp(&pub, &oldpub, sizeof pub) == 0);
    WarshipFleet revealed;
    memset(&revealed, 0x5b, sizeof revealed);
    WarshipFleet oldreveal = revealed;
    EQ(warship_lifecycle_copy_revealed_captain(&g, &revealed),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    CHECK(memcmp(&revealed, &oldreveal, sizeof revealed) == 0);
    EQ(event, WARSHIP_LIFECYCLE_EVENT_DRAW);
    same(&g, &saved);
    EQ(cpu.calls, 0u);
    EQ(h.calls, 0u);
    EQ(c.calls, 0u);
}

static void deployment_and_errors(void)
{
    WarshipLifecycle g = initial();
    WarshipLifecyclePublic pub;
    get(&g, &pub);
    EQ(pub.phase, WARSHIP_LIFECYCLE_HOME);
    EQ(pub.has_human_fleet, 0u);
    EQ(pub.has_battle, 0u);
    EQ(pub.can_reveal_captain, 0u);
    WarshipLifecycle saved = g;
    FleetRng h = left_rng(), c = right_rng();
    EQ(warship_lifecycle_start(&g, NULL, &h, fleet_random, &c),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    same(&g, &saved);
    EQ(warship_lifecycle_start(&g, fleet_random, &h, NULL, &c),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    same(&g, &saved);
    h.fail_at = 2u;
    EQ(warship_lifecycle_start(&g, fleet_random, &h, fleet_random, &c),
       WARSHIP_LIFECYCLE_RANDOM_FAILURE);
    EQ(h.calls, 2u);
    EQ(c.calls, 0u);
    same(&g, &saved);
    h = left_rng();
    c.fail_at = 3u;
    EQ(warship_lifecycle_start(&g, fleet_random, &h, fleet_random, &c),
       WARSHIP_LIFECYCLE_RANDOM_FAILURE);
    EQ(h.calls, 5u);
    EQ(c.calls, 3u);
    same(&g, &saved);
    h = left_rng(); c = right_rng();
    OK(warship_lifecycle_start(&g, fleet_random, &h, fleet_random, &c));
    get(&g, &pub);
    EQ(pub.phase, WARSHIP_LIFECYCLE_DEPLOY);
    EQ(pub.has_human_fleet, 1u);
    EQ(pub.has_battle, 0u);
    EQ(pub.can_reveal_captain, 0u);
    CHECK(memcmp(&pub.human, &g.human, sizeof pub.human) == 0);
    EQ(warship_fleet_validate(&pub.human), WARSHIP_FLEET_OK);
    saved = g;
    h = left_rng(); c = right_rng();
    EQ(warship_lifecycle_start(&g, fleet_random, &h, fleet_random, &c),
       WARSHIP_LIFECYCLE_BAD_PHASE);
    same(&g, &saved);
    EQ(h.calls, 0u); EQ(c.calls, 0u);
    EQ(warship_lifecycle_select(&g, 0u, 0u),
       WARSHIP_LIFECYCLE_BAD_PHASE);
    EQ(warship_lifecycle_fire(&g, cpu_random, NULL, NULL),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    same(&g, &saved);

    /* Matching random formation is NOT an error, even under AUTO. */
    h = left_rng();
    OK(warship_lifecycle_auto(&g, fleet_random, &h));
    EQ(h.calls, 5u);
    CHECK(memcmp(&g.human, &saved.human, sizeof g.human) == 0);
    CHECK(memcmp(&g.captain, &saved.captain, sizeof g.captain) == 0);
    h = other_right_rng();
    OK(warship_lifecycle_auto(&g, fleet_random, &h));
    EQ(warship_fleet_validate(&g.human), WARSHIP_FLEET_OK);
    CHECK(memcmp(&g.human, &saved.human, sizeof g.human) != 0);
    CHECK(memcmp(&g.captain, &saved.captain, sizeof g.captain) == 0);
    for (unsigned i = 0u; i < 6u; ++i) {
        h = (i & 1u) ? left_rng() : other_right_rng();
        OK(warship_lifecycle_auto(&g, fleet_random, &h));
        EQ(warship_fleet_validate(&g.human), WARSHIP_FLEET_OK);
        EQ(h.calls, 5u);
    }
    saved = g;
    h = left_rng(); h.fail_at = 4u;
    EQ(warship_lifecycle_auto(&g, fleet_random, &h),
       WARSHIP_LIFECYCLE_RANDOM_FAILURE);
    same(&g, &saved);
    EQ(warship_lifecycle_auto(&g, NULL, NULL),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    same(&g, &saved);
    OK(warship_lifecycle_leave_deploy(&g));
    get(&g, &pub);
    EQ(pub.phase, WARSHIP_LIFECYCLE_HOME);
    EQ(pub.has_human_fleet, 0u);
    EQ(pub.has_battle, 0u);
    EQ(warship_lifecycle_leave_deploy(&g), WARSHIP_LIFECYCLE_BAD_PHASE);
    /* No outcome event was emitted by any pre-PRÊT transition. */
}

static void forfeiture_and_retries(void)
{
    WarshipLifecycle g = initial();
    ready(&g);
    WarshipLifecyclePublic before, after;
    get(&g, &before);
    EQ(before.phase, WARSHIP_LIFECYCLE_COMBAT);
    EQ(before.has_battle, 1u);
    EQ(before.battle.history.outgoing_count, 0u);
    EQ(before.battle.history.incoming_count, 0u);
    EQ(before.can_reveal_captain, 0u);
    EQ(before.has_human_fleet, 1u);
    EQ(warship_lifecycle_ready(&g), WARSHIP_LIFECYCLE_BAD_PHASE);
    EQ(warship_lifecycle_leave_deploy(&g), WARSHIP_LIFECYCLE_BAD_PHASE);
    EQ(warship_lifecycle_leave_result(&g), WARSHIP_LIFECYCLE_BAD_PHASE);
    EQ(warship_lifecycle_replay(&g, fleet_random, NULL),
       WARSHIP_LIFECYCLE_BAD_PHASE);
    WarshipLifecycle saved = g;
    WarshipLifecycleEvent event = WARSHIP_LIFECYCLE_EVENT_VICTORY;
    EQ(warship_lifecycle_decide_abandon(&g, 1u, &event),
       WARSHIP_LIFECYCLE_BAD_PHASE);
    EQ(event, WARSHIP_LIFECYCLE_EVENT_VICTORY);
    same(&g, &saved);
    CpuRng cpu = {0u, 0, 0u};
    WarshipBattlePair pair;
    memset(&pair, 0xab, sizeof pair);
    WarshipBattlePair pair_old = pair;
    EQ(warship_lifecycle_fire(&g, cpu_random, &cpu, &pair),
       WARSHIP_LIFECYCLE_NO_SELECTION);
    EQ(cpu.calls, 0u);
    same(&g, &saved);
    CHECK(memcmp(&pair, &pair_old, sizeof pair) == 0);
    EQ(warship_lifecycle_select(&g, 10u, 0u),
       WARSHIP_LIFECYCLE_OUT_OF_BOUNDS);
    same(&g, &saved);
    OK(warship_lifecycle_select(&g, 9u, 9u));
    saved = g;
    EQ(warship_lifecycle_select(&g, 9u, 9u),
       WARSHIP_LIFECYCLE_DUPLICATE_SELECTION);
    same(&g, &saved);
    cpu.failure = 1;
    EQ(warship_lifecycle_fire(&g, cpu_random, &cpu, &pair),
       WARSHIP_LIFECYCLE_RANDOM_FAILURE);
    same(&g, &saved);
    CHECK(memcmp(&pair, &pair_old, sizeof pair) == 0);
    cpu.failure = 0;
    OK(warship_lifecycle_fire(&g, cpu_random, &cpu, &pair));
    EQ(cpu.calls, 2u);
    EQ(pair.round, 1u);
    EQ(pair.verdict, WARSHIP_BATTLE_CONTINUE);
    get(&g, &after);
    EQ(after.battle.phase, WARSHIP_SOLO_AWAIT_REVEAL);
    EQ(after.battle.history.outgoing_count, 0u);
    EQ(after.battle.history.incoming_count, 0u);
    saved = g;
    EQ(warship_lifecycle_request_abandon(&g),
       WARSHIP_LIFECYCLE_BAD_PHASE);
    EQ(warship_lifecycle_select(&g, 8u, 8u),
       WARSHIP_LIFECYCLE_BAD_PHASE);
    EQ(warship_lifecycle_fire(&g, cpu_random, &cpu, &pair),
       WARSHIP_LIFECYCLE_ALREADY_FIRED);
    same(&g, &saved);
    EQ(cpu.calls, 2u);
    OK(warship_lifecycle_ack_reveal(&g, &event));
    EQ(event, WARSHIP_LIFECYCLE_EVENT_NONE);
    get(&g, &after);
    EQ(after.battle.history.outgoing_count, 1u);
    EQ(after.battle.history.incoming_count, 1u);
    EQ(after.battle.phase, WARSHIP_SOLO_SELECTING);
    EQ(warship_lifecycle_select(&g, 9u, 9u),
       WARSHIP_LIFECYCLE_ALREADY_ATTACKED);

    OK(warship_lifecycle_select(&g, 8u, 8u));
    saved = g;
    get(&g, &before);
    OK(warship_lifecycle_request_abandon(&g));
    EQ(warship_lifecycle_request_abandon(&g), WARSHIP_LIFECYCLE_BAD_PHASE);
    EQ(warship_lifecycle_select(&g, 7u, 7u),
       WARSHIP_LIFECYCLE_BAD_PHASE);
    EQ(warship_lifecycle_cancel(&g), WARSHIP_LIFECYCLE_BAD_PHASE);
    EQ(warship_lifecycle_fire(&g, cpu_random, &cpu, &pair),
       WARSHIP_LIFECYCLE_BAD_PHASE);
    EQ(warship_lifecycle_ack_reveal(&g, &event),
       WARSHIP_LIFECYCLE_BAD_PHASE);
    EQ(event, WARSHIP_LIFECYCLE_EVENT_NONE);
    get(&g, &after);
    EQ(after.phase, WARSHIP_LIFECYCLE_CONFIRM_ABANDON);
    CHECK(memcmp(&after.battle, &before.battle, sizeof after.battle) == 0);
    WarshipLifecycle pending = g;
    EQ(warship_lifecycle_decide_abandon(&g, 2u, &event),
       WARSHIP_LIFECYCLE_BAD_ARGUMENT);
    same(&g, &pending);
    EQ(event, WARSHIP_LIFECYCLE_EVENT_NONE);
    OK(warship_lifecycle_decide_abandon(&g, 0u, &event));
    EQ(event, WARSHIP_LIFECYCLE_EVENT_NONE);
    CHECK(memcmp(&g.solo, &saved.solo, sizeof g.solo) == 0);
    get(&g, &after);
    CHECK(memcmp(&after.battle, &before.battle, sizeof after.battle) == 0);
    EQ(after.phase, WARSHIP_LIFECYCLE_COMBAT);
    OK(warship_lifecycle_request_abandon(&g));
    OK(warship_lifecycle_decide_abandon(
        &g, 1u, &event));
    EQ(event, WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT);
    get(&g, &after);
    EQ(after.phase, WARSHIP_LIFECYCLE_HOME);
    EQ(after.has_battle, 0u);
    EQ(after.has_human_fleet, 0u);
    pending = g;
    EQ(warship_lifecycle_decide_abandon(&g, 1u, &event),
       WARSHIP_LIFECYCLE_BAD_PHASE);
    same(&g, &pending);
    EQ(event, WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT);
    /* A fresh game is possible, but no event follows merely from START. */
    deployed(&g);
    OK(warship_lifecycle_leave_deploy(&g));
}

static const uint8_t ship_lengths[5] = {2u, 3u, 3u, 4u, 5u};
static void ship_cell(unsigned ordinal, unsigned offset,
                      uint8_t *x, uint8_t *y)
{
    for (unsigned id = 0u; id < 5u; ++id) {
        if (ordinal < ship_lengths[id]) {
            *x = (uint8_t)(offset + ordinal);
            *y = (uint8_t)id;
            return;
        }
        ordinal -= ship_lengths[id];
    }
    CHECK(0);
}
static int captain_occupied(unsigned x, unsigned y)
{
    return y < 5u && x >= 5u && x < 5u + ship_lengths[y];
}
static void water_cell(unsigned ordinal, uint8_t *x, uint8_t *y)
{
    for (unsigned cell = 0u; cell < 100u; ++cell) {
        if (captain_occupied(cell % 10u, cell / 10u)) continue;
        if (ordinal == 0u) {
            *x = (uint8_t)(cell % 10u);
            *y = (uint8_t)(cell / 10u);
            return;
        }
        --ordinal;
    }
    CHECK(0);
}
static unsigned ending(unsigned mode, unsigned draw_at,
                       WarshipBattleShot trace[100])
{
    WarshipLifecycle g = initial();
    ready(&g);
    CpuRng cpu = {0u, 0, 0u};
    WarshipLifecyclePublic pub;
    WarshipLifecycleEvent expected = mode == 0u ?
        WARSHIP_LIFECYCLE_EVENT_VICTORY : mode == 1u ?
        WARSHIP_LIFECYCLE_EVENT_DEFEAT : WARSHIP_LIFECYCLE_EVENT_DRAW;
    unsigned completed = 0u;
    for (unsigned n = 0u; n < 100u; ++n) {
        uint8_t x, y;
        unsigned hit = mode == 0u || n < 16u ||
                       (mode == 2u && n + 1u == draw_at);
        if (hit) ship_cell(n < 16u ? n : 16u, 5u, &x, &y);
        else water_cell(n - 16u, &x, &y);
        OK(warship_lifecycle_select(&g, x, y));
        WarshipBattlePair pair;
        memset(&pair, 0, sizeof pair);
        OK(warship_lifecycle_fire(&g, cpu_random, &cpu, &pair));
        EQ(pair.round, n + 1u);
        EQ(cpu.calls, n + 1u);
        if (mode == 1u) trace[n] = pair.captain;
        if (mode == 2u) {
            EQ(pair.captain.x, trace[n].x);
            EQ(pair.captain.y, trace[n].y);
            EQ(pair.captain.impact, trace[n].impact);
        }
        get(&g, &pub);
        EQ(pub.phase, WARSHIP_LIFECYCLE_COMBAT);
        EQ(pub.battle.history.outgoing_count, n);
        EQ(pub.battle.history.incoming_count, n);
        EQ(pub.battle.history.verdict, WARSHIP_BATTLE_CONTINUE);
        WarshipLifecycleEvent event = WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT;
        OK(warship_lifecycle_ack_reveal(&g, &event));
        get(&g, &pub);
        EQ(pub.battle.history.outgoing_count, n + 1u);
        EQ(pub.battle.history.incoming_count, n + 1u);
        EQ(pub.battle.history.verdict, pair.verdict);
        if (pair.verdict == WARSHIP_BATTLE_CONTINUE) {
            EQ(event, WARSHIP_LIFECYCLE_EVENT_NONE);
            EQ(pub.phase, WARSHIP_LIFECYCLE_COMBAT);
            EQ(pub.battle.phase, WARSHIP_SOLO_SELECTING);
            continue;
        }
        EQ(event, expected);
        EQ(pub.phase, WARSHIP_LIFECYCLE_RESULT);
        EQ(pub.battle.phase, WARSHIP_SOLO_FINISHED);
        EQ(pub.can_reveal_captain, mode == 1u ? 1u : 0u);
        if (mode == 0u || mode == 2u) {
            EQ(pair.human.impact, WARSHIP_BATTLE_SUNK);
            EQ(pair.human.ship_id, WARSHIP_SHIP_CARRIER);
        }
        if (mode == 1u || mode == 2u) {
            EQ(pair.captain.impact, WARSHIP_BATTLE_SUNK);
            CHECK(pair.captain.ship_id < WARSHIP_FLEET_SHIPS);
        }
        WarshipLifecycle before = g;
        EQ(warship_lifecycle_ack_reveal(&g, &event),
           WARSHIP_LIFECYCLE_BAD_PHASE);
        EQ(event, expected);
        same(&g, &before);
        EQ(warship_lifecycle_select(&g, 9u, 9u),
           WARSHIP_LIFECYCLE_BAD_PHASE);
        EQ(warship_lifecycle_request_abandon(&g),
           WARSHIP_LIFECYCLE_BAD_PHASE);
        WarshipFleet reveal;
        memset(&reveal, 0xd2, sizeof reveal);
        WarshipFleet old = reveal;
        EQ(warship_lifecycle_copy_revealed_captain(&g, &reveal),
           WARSHIP_LIFECYCLE_REVEAL_DENIED);
        CHECK(memcmp(&reveal, &old, sizeof reveal) == 0);
        if (mode == 1u) {
            EQ(pub.reveal_visible, 0u);
            EQ(pub.reveal_requested, 0u);
            EQ(warship_lifecycle_set_captain_visible(&g, 0u),
               WARSHIP_LIFECYCLE_REVEAL_DENIED);
            OK(warship_lifecycle_set_captain_visible(&g, 1u));
            get(&g, &pub);
            EQ(pub.reveal_requested, 1u);
            EQ(pub.reveal_visible, 1u);
            OK(warship_lifecycle_copy_revealed_captain(&g, &reveal));
            CHECK(memcmp(&reveal, &g.captain, sizeof reveal) == 0);
            OK(warship_lifecycle_set_captain_visible(&g, 0u));
            get(&g, &pub);
            EQ(pub.reveal_requested, 1u);
            EQ(pub.reveal_visible, 0u);
            EQ(warship_lifecycle_copy_revealed_captain(&g, &reveal),
               WARSHIP_LIFECYCLE_REVEAL_DENIED);
            OK(warship_lifecycle_set_captain_visible(&g, 1u));
        } else {
            EQ(warship_lifecycle_set_captain_visible(&g, 1u),
               WARSHIP_LIFECYCLE_REVEAL_DENIED);
        }
        completed = n + 1u;
        const WarshipFleet human = g.human, previous_cpu = g.captain;
        FleetRng wrong = other_right_rng();
        wrong.fail_at = 2u;
        before = g;
        EQ(warship_lifecycle_replay(&g, fleet_random, &wrong),
           WARSHIP_LIFECYCLE_RANDOM_FAILURE);
        same(&g, &before);
        wrong = other_right_rng();
        OK(warship_lifecycle_replay(&g, fleet_random, &wrong));
        EQ(wrong.calls, 5u);
        EQ(warship_fleet_validate(&g.human), WARSHIP_FLEET_OK);
        CHECK(memcmp(&g.human, &human, sizeof human) == 0);
        CHECK(memcmp(&g.captain, &previous_cpu, sizeof previous_cpu) != 0);
        get(&g, &pub);
        EQ(pub.phase, WARSHIP_LIFECYCLE_DEPLOY);
        EQ(pub.has_battle, 0u);
        EQ(pub.reveal_requested, 0u);
        EQ(pub.reveal_visible, 0u);
        EQ(pub.can_reveal_captain, 0u);
        OK(warship_lifecycle_ready(&g));
        get(&g, &pub);
        EQ(pub.battle.history.outgoing_count, 0u);
        EQ(pub.battle.history.incoming_count, 0u);
        EQ(pub.battle.history.verdict, WARSHIP_BATTLE_CONTINUE);
        OK(warship_lifecycle_select(&g, 9u, 9u));
        WarshipBattlePair next_pair;
        OK(warship_lifecycle_fire(&g, cpu_random, &cpu, &next_pair));
        EQ(next_pair.round, 1u);
        break;
    }
    CHECK(completed != 0u);
    return completed;
}

static void after_result_home(void)
{
    WarshipLifecycle g = initial();
    ready(&g);
    /* Leaving COMBAT directly is not a free departure or automatic
     * forfeiture; only explicit confirmation can do that. */
    EQ(warship_lifecycle_leave_result(&g), WARSHIP_LIFECYCLE_BAD_PHASE);
    EQ(warship_lifecycle_leave_deploy(&g), WARSHIP_LIFECYCLE_BAD_PHASE);
    WarshipLifecyclePublic pub;
    get(&g, &pub);
    EQ(pub.can_reveal_captain, 0u);
    WarshipFleet tmp;
    EQ(warship_lifecycle_copy_revealed_captain(&g, &tmp),
       WARSHIP_LIFECYCLE_REVEAL_DENIED);
}

int main(void)
{
    bad_arguments();
    deployment_and_errors();
    forfeiture_and_retries();
    after_result_home();
    WarshipBattleShot trace[100] = {{0}};
    EQ(ending(0u, 0u, trace), 17u);
    const unsigned defeat_round = ending(1u, 0u, trace);
    CHECK(defeat_round >= 17u && defeat_round <= 99u);
    EQ(ending(2u, defeat_round, trace), defeat_round);
    printf("D07 PASS sizeof(lifecycle)=%zu sizeof(ui)=%zu\n",
           sizeof(WarshipLifecycle), sizeof(WarshipLifecyclePublic));
    return EXIT_SUCCESS;
}
