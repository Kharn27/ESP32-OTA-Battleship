#include "warship_solo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(c) do { if (!(c)) { \
    fprintf(stderr, "FAIL %s:%d %s\\n", __FILE__, __LINE__, #c); \
    exit(EXIT_FAILURE); \
} } while (0)
#define EQ(a,b) CHECK((a) == (b))
#define OK(expr) EQ((expr), WARSHIP_SOLO_OK)

static const WarshipFleet fleet_left = {{
    {WARSHIP_SHIP_PATROL, 0u, 0u, 2u, WARSHIP_ORIENT_HORIZONTAL},
    {WARSHIP_SHIP_SUBMARINE, 0u, 1u, 3u, WARSHIP_ORIENT_HORIZONTAL},
    {WARSHIP_SHIP_DESTROYER, 0u, 2u, 3u, WARSHIP_ORIENT_HORIZONTAL},
    {WARSHIP_SHIP_CRUISER, 0u, 3u, 4u, WARSHIP_ORIENT_HORIZONTAL},
    {WARSHIP_SHIP_CARRIER, 0u, 4u, 5u, WARSHIP_ORIENT_HORIZONTAL}
}};
static const WarshipFleet fleet_right = {{
    {WARSHIP_SHIP_PATROL, 5u, 0u, 2u, WARSHIP_ORIENT_HORIZONTAL},
    {WARSHIP_SHIP_SUBMARINE, 5u, 1u, 3u, WARSHIP_ORIENT_HORIZONTAL},
    {WARSHIP_SHIP_DESTROYER, 5u, 2u, 3u, WARSHIP_ORIENT_HORIZONTAL},
    {WARSHIP_SHIP_CRUISER, 5u, 3u, 4u, WARSHIP_ORIENT_HORIZONTAL},
    {WARSHIP_SHIP_CARRIER, 5u, 4u, 5u, WARSHIP_ORIENT_HORIZONTAL}
}};
static const uint8_t lengths[5] = {2u, 3u, 3u, 4u, 5u};

typedef struct {
    uint32_t value;
    unsigned calls;
    int fail;
} TestRng;
static int sample(void *context, uint32_t *out)
{
    TestRng *rng = (TestRng *)context;
    ++rng->calls;
    if (rng->fail) return -1;
    *out = rng->value;
    return 0;
}

static WarshipSoloSession init(const WarshipFleet *human,
                               const WarshipFleet *captain)
{
    WarshipSoloSession session;
    memset(&session, 0xa5, sizeof session);
    OK(warship_solo_init(&session, human, captain));
    return session;
}
static void unchanged(const WarshipSoloSession *session,
                      const WarshipSoloSession *before)
{
    CHECK(memcmp(session, before, sizeof *session) == 0);
}
static void shot_equal(WarshipBattleShot a, WarshipBattleShot b)
{
    EQ(a.x, b.x);
    EQ(a.y, b.y);
    EQ(a.impact, b.impact);
    EQ(a.ship_id, b.ship_id);
}
static void test_invalid(void)
{
    WarshipSoloSession session;
    memset(&session, 0x9c, sizeof session);
    WarshipSoloSession old = session;
    EQ(warship_solo_init(NULL, &fleet_left, &fleet_right),
       WARSHIP_SOLO_BAD_ARGUMENT);
    EQ(warship_solo_init(&session, NULL, &fleet_right),
       WARSHIP_SOLO_BAD_ARGUMENT);
    unchanged(&session, &old);
    EQ(warship_solo_init(&session, &fleet_left, NULL),
       WARSHIP_SOLO_BAD_ARGUMENT);
    unchanged(&session, &old);
    WarshipFleet bad = fleet_left;
    bad.ships[0].x = 10u;
    EQ(warship_solo_init(&session, &bad, &fleet_right),
       WARSHIP_SOLO_BAD_FLEET);
    unchanged(&session, &old);
    EQ(warship_solo_init(&session, &fleet_right, &bad),
       WARSHIP_SOLO_BAD_FLEET);
    unchanged(&session, &old);

    WarshipSoloPublic public_state;
    memset(&public_state, 0x55, sizeof public_state);
    WarshipSoloPublic prior = public_state;
    EQ(warship_solo_observe(&session, &public_state),
       WARSHIP_SOLO_BAD_ARGUMENT);
    CHECK(memcmp(&public_state, &prior, sizeof prior) == 0);
    EQ(warship_solo_select(&session, 0u, 0u), WARSHIP_SOLO_BAD_ARGUMENT);
    EQ(warship_solo_cancel(&session), WARSHIP_SOLO_BAD_ARGUMENT);
    WarshipBattlePair pair;
    memset(&pair, 0x3c, sizeof pair);
    EQ(warship_solo_fire(&session, sample, NULL, &pair),
       WARSHIP_SOLO_BAD_ARGUMENT);
    EQ(warship_solo_ack_reveal(&session), WARSHIP_SOLO_BAD_ARGUMENT);
    WarshipBattleVerdict verdict = WARSHIP_BATTLE_DRAW;
    EQ(warship_solo_take_final(&session, &verdict),
       WARSHIP_SOLO_BAD_ARGUMENT);
    unchanged(&session, &old);
    EQ(verdict, WARSHIP_BATTLE_DRAW);

    session = init(&fleet_left, &fleet_right);
    old = session;
    EQ(warship_solo_observe(NULL, &public_state),
       WARSHIP_SOLO_BAD_ARGUMENT);
    EQ(warship_solo_observe(&session, NULL),
       WARSHIP_SOLO_BAD_ARGUMENT);
    EQ(warship_solo_fire(&session, NULL, NULL, &pair),
       WARSHIP_SOLO_BAD_ARGUMENT);
    EQ(warship_solo_fire(&session, sample, NULL, NULL),
       WARSHIP_SOLO_BAD_ARGUMENT);
    EQ(warship_solo_take_final(&session, NULL),
       WARSHIP_SOLO_BAD_ARGUMENT);
    unchanged(&session, &old);
    CHECK(memcmp(&pair, &(WarshipBattlePair){0}, 0u) == 0); /* no read */
}

static void test_selection_and_atomic_fire(void)
{
    WarshipSoloSession session = init(&fleet_left, &fleet_right);
    WarshipSoloPublic pub;
    OK(warship_solo_observe(&session, &pub));
    EQ(pub.phase, WARSHIP_SOLO_SELECTING);
    EQ(pub.has_selection, 0u);
    EQ(pub.history.outgoing_count, 0u);
    EQ(pub.history.incoming_count, 0u);

    WarshipSoloSession before = session;
    EQ(warship_solo_select(&session, 10u, 0u), WARSHIP_SOLO_OUT_OF_BOUNDS);
    unchanged(&session, &before);
    EQ(warship_solo_cancel(&session), WARSHIP_SOLO_NO_SELECTION);
    unchanged(&session, &before);
    TestRng rng = {0u, 0u, 0};
    WarshipBattlePair pair;
    memset(&pair, 0x6d, sizeof pair);
    WarshipBattlePair oldpair = pair;
    EQ(warship_solo_fire(&session, sample, &rng, &pair),
       WARSHIP_SOLO_NO_SELECTION);
    EQ(rng.calls, 0u);
    unchanged(&session, &before);
    CHECK(memcmp(&pair, &oldpair, sizeof pair) == 0);

    OK(warship_solo_select(&session, 8u, 8u));
    before = session;
    EQ(warship_solo_select(&session, 8u, 8u),
       WARSHIP_SOLO_DUPLICATE_SELECTION);
    unchanged(&session, &before);
    OK(warship_solo_select(&session, 9u, 9u));
    OK(warship_solo_observe(&session, &pub));
    EQ(pub.has_selection, 1u);
    EQ(pub.target_x, 9u);
    EQ(pub.target_y, 9u);
    OK(warship_solo_cancel(&session));
    OK(warship_solo_observe(&session, &pub));
    EQ(pub.has_selection, 0u);
    OK(warship_solo_select(&session, 9u, 9u));

    /* Failed RNG must not irreversibly lock the human FEU. */
    rng.fail = 1;
    before = session;
    EQ(warship_solo_fire(&session, sample, &rng, &pair),
       WARSHIP_SOLO_RANDOM_FAILURE);
    EQ(rng.calls, 1u);
    unchanged(&session, &before);
    CHECK(memcmp(&pair, &oldpair, sizeof pair) == 0);
    OK(warship_solo_observe(&session, &pub));
    EQ(pub.has_selection, 1u);
    EQ(pub.target_x, 9u);
    OK(warship_solo_select(&session, 8u, 8u));
    OK(warship_solo_select(&session, 9u, 9u));

    rng.fail = 0;
    OK(warship_solo_fire(&session, sample, &rng, &pair));
    EQ(rng.calls, 2u);
    EQ(pair.round, 1u);
    EQ(pair.verdict, WARSHIP_BATTLE_CONTINUE);
    EQ(pair.human.x, 9u);
    EQ(pair.human.y, 9u);
    EQ(pair.human.impact, WARSHIP_BATTLE_MISS);
    before = session;
    WarshipBattlePair pair_before = pair;
    EQ(warship_solo_fire(&session, sample, &rng, &pair),
       WARSHIP_SOLO_ALREADY_FIRED);
    EQ(warship_solo_select(&session, 5u, 0u), WARSHIP_SOLO_BAD_PHASE);
    EQ(warship_solo_cancel(&session), WARSHIP_SOLO_BAD_PHASE);
    EQ(warship_solo_ack_reveal(NULL), WARSHIP_SOLO_BAD_ARGUMENT);
    EQ(rng.calls, 2u);
    unchanged(&session, &before);
    CHECK(memcmp(&pair, &pair_before, sizeof pair) == 0);

    /* No new history or terminal outcome leaks until presentation ACK. */
    OK(warship_solo_observe(&session, &pub));
    EQ(pub.phase, WARSHIP_SOLO_AWAIT_REVEAL);
    EQ(pub.has_selection, 0u);
    EQ(pub.history.outgoing_count, 0u);
    EQ(pub.history.incoming_count, 0u);
    EQ(pub.history.verdict, WARSHIP_BATTLE_CONTINUE);
    EQ(pub.history.outgoing[0].impact, 0u);
    EQ(pub.history.incoming[0].impact, 0u);
    OK(warship_solo_ack_reveal(&session));
    EQ(warship_solo_ack_reveal(&session), WARSHIP_SOLO_BAD_PHASE);
    OK(warship_solo_observe(&session, &pub));
    EQ(pub.phase, WARSHIP_SOLO_SELECTING);
    EQ(pub.history.outgoing_count, 1u);
    EQ(pub.history.incoming_count, 1u);
    shot_equal(pub.history.outgoing[0], pair.human);
    shot_equal(pub.history.incoming[0], pair.captain);

    before = session;
    EQ(warship_solo_select(&session, 9u, 9u),
       WARSHIP_SOLO_ALREADY_ATTACKED);
    unchanged(&session, &before);
    OK(warship_solo_select(&session, 5u, 0u));
    OK(warship_solo_fire(&session, sample, &rng, &pair));
    EQ(pair.round, 2u);
    OK(warship_solo_ack_reveal(&session));
}

static void test_no_cheat(void)
{
    WarshipSoloSession a = init(&fleet_left, &fleet_right);
    WarshipSoloSession b = init(&fleet_right, &fleet_right);
    /* Different private human fleets, and different unpublished human
     * targets, cannot reach the CPU's public observation. */
    OK(warship_solo_select(&a, 9u, 9u));
    OK(warship_solo_select(&b, 8u, 8u));
    WarshipSoloPublic pa, pb;
    OK(warship_solo_observe(&a, &pa));
    OK(warship_solo_observe(&b, &pb));
    CHECK(memcmp(&pa.history, &pb.history, sizeof pa.history) == 0);
    TestRng ra = {UINT32_C(812731), 0u, 0};
    TestRng rb = ra;
    WarshipBattlePair ap, bp;
    OK(warship_solo_fire(&a, sample, &ra, &ap));
    OK(warship_solo_fire(&b, sample, &rb, &bp));
    EQ(ra.calls, 1u);
    EQ(rb.calls, 1u);
    EQ(ap.captain.x, bp.captain.x);
    EQ(ap.captain.y, bp.captain.y);
    OK(warship_solo_observe(&a, &pa));
    OK(warship_solo_observe(&b, &pb));
    CHECK(memcmp(&pa.history, &pb.history, sizeof pa.history) == 0);
    OK(warship_solo_ack_reveal(&a));
    OK(warship_solo_ack_reveal(&b));
    OK(warship_solo_observe(&a, &pa));
    OK(warship_solo_observe(&b, &pb));
    /* Human-facing projection has only public shot histories. */
    EQ(pa.history.outgoing_count, 1u);
    EQ(pb.history.outgoing_count, 1u);
}

static unsigned ship_cell(unsigned ordinal, unsigned offset,
                          uint8_t *x, uint8_t *y)
{
    for (unsigned id = 0u; id < WARSHIP_FLEET_SHIPS; ++id) {
        if (ordinal < lengths[id]) {
            *x = (uint8_t)(offset + ordinal);
            *y = (uint8_t)id;
            return id;
        }
        ordinal -= lengths[id];
    }
    CHECK(0);
    return 0u;
}
static int captain_occupied(unsigned x, unsigned y)
{
    return y < 5u && x >= 5u && x < 5u + lengths[y];
}
static void water_cell(unsigned ordinal, uint8_t *x, uint8_t *y)
{
    for (unsigned i = 0u; i < 100u; ++i) {
        if (captain_occupied(i % 10u, i / 10u)) continue;
        if (ordinal == 0u) {
            *x = (uint8_t)(i % 10u);
            *y = (uint8_t)(i / 10u);
            return;
        }
        --ordinal;
    }
    CHECK(0);
}

/* Three ending scenarios use REAL D05 decisions + REAL D02 resolution,
 * not any injected false state, CPU secret or manually chosen CPU targets.
 * When CPU wins a reference replay, the human can delay just the final
 * carrier hit until exactly that same round -> true simultaneous DRAW.
 */
static unsigned test_endings(int mode, unsigned draw_at,
                              WarshipBattleShot cpu_trace[100])
{
    WarshipSoloSession session = init(&fleet_left, &fleet_right);
    TestRng rng = {0u, 0u, 0};
    unsigned finished_round = 0u;
    for (unsigned n = 0u; n < 100u; ++n) {
        uint8_t x, y;
        unsigned human_hit;
        if (mode == 1) human_hit = 1u;  /* VICTORY: 17 early hits */
        else if (mode == 2) human_hit = n < 16u; /* DEFEAT: last held */
        else human_hit = n < 16u || n + 1u == draw_at;

        if (human_hit)
            (void)ship_cell(n < 16u ? n : 16u, 5u, &x, &y);
        else
            water_cell(n - 16u, &x, &y);
        OK(warship_solo_select(&session, x, y));
        WarshipBattlePair pair = {{0}};
        OK(warship_solo_fire(&session, sample, &rng, &pair));
        EQ(pair.round, n + 1u);
        EQ(rng.calls, n + 1u);
        if (mode == 3) {
            EQ(pair.captain.x, cpu_trace[n].x);
            EQ(pair.captain.y, cpu_trace[n].y);
            EQ(pair.captain.impact, cpu_trace[n].impact);
        }
        if (mode == 2) cpu_trace[n] = pair.captain;
        WarshipSoloPublic before_reveal;
        OK(warship_solo_observe(&session, &before_reveal));
        EQ(before_reveal.phase, WARSHIP_SOLO_AWAIT_REVEAL);
        EQ(before_reveal.history.outgoing_count, n);
        EQ(before_reveal.history.incoming_count, n);
        EQ(before_reveal.history.verdict, WARSHIP_BATTLE_CONTINUE);
        WarshipBattleVerdict v = WARSHIP_BATTLE_CONTINUE;
        EQ(warship_solo_take_final(&session, &v), WARSHIP_SOLO_NO_EVENT);
        EQ(v, WARSHIP_BATTLE_CONTINUE);
        OK(warship_solo_ack_reveal(&session));
        WarshipSoloPublic after_reveal;
        OK(warship_solo_observe(&session, &after_reveal));
        EQ(after_reveal.history.outgoing_count, n + 1u);
        EQ(after_reveal.history.incoming_count, n + 1u);
        shot_equal(after_reveal.history.outgoing[n], pair.human);
        shot_equal(after_reveal.history.incoming[n], pair.captain);
        EQ(after_reveal.history.verdict, pair.verdict);
        if (pair.verdict != WARSHIP_BATTLE_CONTINUE) {
            const unsigned expected = mode == 1 ?
                WARSHIP_BATTLE_VICTORY : mode == 2 ?
                WARSHIP_BATTLE_DEFEAT : WARSHIP_BATTLE_DRAW;
            EQ(pair.verdict, expected);
            EQ(after_reveal.phase, WARSHIP_SOLO_FINISHED);
            EQ(warship_solo_select(&session, 9u, 9u),
               WARSHIP_SOLO_BAD_PHASE);
            EQ(warship_solo_cancel(&session), WARSHIP_SOLO_BAD_PHASE);
            EQ(warship_solo_fire(&session, sample, &rng, &pair),
               WARSHIP_SOLO_BAD_PHASE);
            EQ(warship_solo_ack_reveal(&session), WARSHIP_SOLO_BAD_PHASE);
            EQ(rng.calls, n + 1u);
            OK(warship_solo_take_final(&session, &v));
            EQ(v, expected);
            const WarshipSoloSession consumed = session;
            v = WARSHIP_BATTLE_CONTINUE;
            EQ(warship_solo_take_final(&session, &v), WARSHIP_SOLO_NO_EVENT);
            unchanged(&session, &consumed);
            EQ(v, WARSHIP_BATTLE_CONTINUE);
            finished_round = n + 1u;
            break;
        }
        EQ(after_reveal.phase, WARSHIP_SOLO_SELECTING);
    }
    CHECK(finished_round != 0u);
    return finished_round;
}

static void test_multiround_and_terminal(void)
{
    WarshipBattleShot cpu_trace[100] = {{0}};
    /* All 17 Capitaine ship cells are sunk in exactly 17 rounds.
     * CPU has no time to discover and sink 17 human cells in this run. */
    EQ(test_endings(1, 0u, cpu_trace), 17u);
    const unsigned defeated_at = test_endings(2, 0u, cpu_trace);
    CHECK(defeated_at >= 17u && defeated_at <= 99u);
    EQ(test_endings(3, defeated_at, cpu_trace), defeated_at);
}
int main(void)
{
    test_invalid();
    test_selection_and_atomic_fire();
    test_no_cheat();
    test_multiround_and_terminal();
    printf("D06 PASS sizeof(session)=%zu sizeof(public)=%zu\\n",
           sizeof(WarshipSoloSession), sizeof(WarshipSoloPublic));
    return 0;
}
