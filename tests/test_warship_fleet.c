#include "warship_fleet.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s failed\n", __FILE__, __LINE__, #expr); \
    exit(EXIT_FAILURE); \
} } while (0)

static const WarshipFleet valid = {{
    {WARSHIP_SHIP_PATROL,     0u, 0u, 2u, WARSHIP_ORIENT_HORIZONTAL},
    {WARSHIP_SHIP_SUBMARINE,  2u, 0u, 3u, WARSHIP_ORIENT_VERTICAL},
    {WARSHIP_SHIP_DESTROYER,  3u, 0u, 3u, WARSHIP_ORIENT_HORIZONTAL},
    {WARSHIP_SHIP_CRUISER,   6u, 0u, 4u, WARSHIP_ORIENT_VERTICAL},
    {WARSHIP_SHIP_CARRIER,   5u, 5u, 5u, WARSHIP_ORIENT_HORIZONTAL}
}};

typedef struct { uint32_t state; unsigned requests; unsigned fail_at; } Rng;

static int seeded(void *context, uint32_t *value)
{
    Rng *rng = (Rng *)context;
    ++rng->requests;
    if (rng->fail_at != 0u && rng->requests == rng->fail_at)
        return -1;
    rng->state ^= rng->state << 13;
    rng->state ^= rng->state >> 17;
    rng->state ^= rng->state << 5;
    *value = rng->state;
    return 0;
}

static int constant(void *context, uint32_t *value)
{
    *value = *(const uint32_t *)context;
    return 0;
}

static void verify_contents(const WarshipFleet *fleet)
{
    uint8_t used[100] = {0};
    unsigned cells = 0u;
    unsigned ids = 0u;
    CHECK(warship_fleet_validate(fleet) == WARSHIP_FLEET_OK);
    for (unsigned i = 0u; i < WARSHIP_FLEET_SHIPS; ++i) {
        const WarshipShip *ship = &fleet->ships[i];
        const unsigned bit = 1u << ship->id;
        CHECK((ids & bit) == 0u);
        ids |= bit;
        CHECK(ship->length == warship_fleet_ship_length(ship->id));
        for (unsigned k = 0u; k < ship->length; ++k) {
            const unsigned x = (unsigned)ship->x +
                (ship->orientation == WARSHIP_ORIENT_HORIZONTAL ? k : 0u);
            const unsigned y = (unsigned)ship->y +
                (ship->orientation == WARSHIP_ORIENT_VERTICAL ? k : 0u);
            CHECK(x < 10u && y < 10u);
            CHECK(used[y * 10u + x] == 0u);
            used[y * 10u + x] = 1u;
            ++cells;
        }
    }
    CHECK(ids == 31u);
    CHECK(cells == WARSHIP_FLEET_CELLS);
}

static void geometry_tests(void)
{
    CHECK(sizeof (WarshipShip) == 5u);
    CHECK(sizeof (WarshipFleet) == 25u);
    CHECK(warship_fleet_ship_length(0u) == 2u);
    CHECK(warship_fleet_ship_length(1u) == 3u);
    CHECK(warship_fleet_ship_length(2u) == 3u);
    CHECK(warship_fleet_ship_length(3u) == 4u);
    CHECK(warship_fleet_ship_length(4u) == 5u);
    CHECK(warship_fleet_ship_length(5u) == 0u);
    CHECK(warship_fleet_ship_length(255u) == 0u);
    CHECK(warship_fleet_validate(NULL) == WARSHIP_FLEET_INVALID_ARGUMENT);
    verify_contents(&valid); /* contacts by side and diagonal */

    WarshipFleet fleet = valid;
    fleet.ships[0].id = WARSHIP_SHIP_SUBMARINE;
    CHECK(warship_fleet_validate(&fleet) == WARSHIP_FLEET_INVALID_LENGTH);
    fleet = valid;
    fleet.ships[1].id = WARSHIP_SHIP_DESTROYER;
    CHECK(warship_fleet_validate(&fleet) == WARSHIP_FLEET_DUPLICATE_ID);
    fleet = valid;
    fleet.ships[0].id = 5u;
    CHECK(warship_fleet_validate(&fleet) == WARSHIP_FLEET_INVALID_ID);
    fleet = valid;
    fleet.ships[1].length = 2u;
    CHECK(warship_fleet_validate(&fleet) == WARSHIP_FLEET_INVALID_LENGTH);
    fleet = valid;
    fleet.ships[1].length = 0u;
    CHECK(warship_fleet_validate(&fleet) == WARSHIP_FLEET_INVALID_LENGTH);
    fleet = valid;
    fleet.ships[1].orientation = 2u;
    CHECK(warship_fleet_validate(&fleet) == WARSHIP_FLEET_INVALID_ORIENTATION);
    fleet = valid;
    fleet.ships[0].x = 9u;
    CHECK(warship_fleet_validate(&fleet) == WARSHIP_FLEET_OUT_OF_BOUNDS);
    fleet = valid;
    fleet.ships[1].y = 9u;
    CHECK(warship_fleet_validate(&fleet) == WARSHIP_FLEET_OUT_OF_BOUNDS);
    fleet = valid;
    fleet.ships[0].x = 10u;
    CHECK(warship_fleet_validate(&fleet) == WARSHIP_FLEET_OUT_OF_BOUNDS);
    fleet = valid;
    fleet.ships[0].y = 255u;
    CHECK(warship_fleet_validate(&fleet) == WARSHIP_FLEET_OUT_OF_BOUNDS);
    fleet = valid;
    fleet.ships[2].x = 0u;
    CHECK(warship_fleet_validate(&fleet) == WARSHIP_FLEET_OVERLAP);
    fleet = valid;
    fleet.ships[4].x = 5u;
    fleet.ships[4].y = 9u; /* bottom edge, horizontal */
    verify_contents(&fleet);
    fleet = valid;
    fleet.ships[4].x = 9u;
    fleet.ships[4].y = 5u;
    fleet.ships[4].orientation = WARSHIP_ORIENT_VERTICAL;
    verify_contents(&fleet); /* vertical at lower-right */
}

static void copying_tests(void)
{
    WarshipFleet dst;
    memset(&dst, 0xa5, sizeof dst);
    WarshipFleet old = dst;
    CHECK(warship_fleet_copy(NULL, &valid) == WARSHIP_FLEET_INVALID_ARGUMENT);
    CHECK(warship_fleet_copy(&dst, NULL) == WARSHIP_FLEET_INVALID_ARGUMENT);
    CHECK(memcmp(&dst, &old, sizeof dst) == 0);
    WarshipFleet bad = valid;
    bad.ships[0].x = 255u;
    CHECK(warship_fleet_copy(&dst, &bad) == WARSHIP_FLEET_OUT_OF_BOUNDS);
    CHECK(memcmp(&dst, &old, sizeof dst) == 0);
    CHECK(warship_fleet_copy(&dst, &valid) == WARSHIP_FLEET_OK);
    CHECK(memcmp(&dst, &valid, sizeof dst) == 0);
    CHECK(warship_fleet_copy(&dst, &dst) == WARSHIP_FLEET_OK);
    CHECK(memcmp(&dst, &valid, sizeof dst) == 0);
    dst.ships[0].x = 9u;
    CHECK(warship_fleet_validate(&valid) == WARSHIP_FLEET_OK);
}

static void auto_tests(void)
{
    WarshipFleet out, other, backup;
    uint32_t zero = 0u, maximum = UINT32_MAX;
    memset(&out, 0x5a, sizeof out);
    backup = out;
    Rng rng = {0x12345678u, 0u, 0u};
    CHECK(warship_fleet_generate(NULL, seeded, &rng) == WARSHIP_FLEET_INVALID_ARGUMENT);
    CHECK(warship_fleet_generate(&out, NULL, &rng) == WARSHIP_FLEET_INVALID_ARGUMENT);
    CHECK(memcmp(&out, &backup, sizeof out) == 0);
    CHECK(rng.requests == 0u);
    CHECK(warship_fleet_generate(&out, constant, &zero) == WARSHIP_FLEET_OK);
    verify_contents(&out); /* adversarial RNG constant zero */
    CHECK(warship_fleet_generate(&out, constant, &maximum) == WARSHIP_FLEET_OK);
    verify_contents(&out); /* adversarial RNG constant UINT32_MAX */

    unsigned distinct = 0u, horizontal = 0u, vertical = 0u;
    WarshipFleet first = {{{0}}};
    for (uint32_t seed = 1u; seed <= 10000u; ++seed) {
        Rng a = {seed, 0u, 0u};
        Rng b = {seed, 0u, 0u};
        CHECK(warship_fleet_generate(&out, seeded, &a) == WARSHIP_FLEET_OK);
        CHECK(warship_fleet_generate(&other, seeded, &b) == WARSHIP_FLEET_OK);
        CHECK(a.requests == b.requests && a.requests <= 5u);
        CHECK(memcmp(&out, &other, sizeof out) == 0); /* seed replay */
        verify_contents(&out);
        if (seed == 1u) first = out;
        else if (memcmp(&out, &first, sizeof out) != 0) ++distinct;
        for (unsigned j = 0u; j < WARSHIP_FLEET_SHIPS; ++j) {
            horizontal += out.ships[j].orientation == WARSHIP_ORIENT_HORIZONTAL;
            vertical += out.ships[j].orientation == WARSHIP_ORIENT_VERTICAL;
        }
    }
    CHECK(distinct > 5000u);
    CHECK(horizontal > 0u && vertical > 0u);
    printf("AUTO 10000 deterministic seeds; non-first layouts=%u H=%u V=%u\n",
           distinct, horizontal, vertical);

    for (unsigned fail = 1u; fail <= 5u; ++fail) {
        rng = (Rng){0x89abcdefu, 0u, fail};
        memset(&out, 0x5a, sizeof out);
        backup = out;
        CHECK(warship_fleet_generate(&out, seeded, &rng) == WARSHIP_FLEET_RANDOM_FAILURE);
        CHECK(rng.requests == fail);
        CHECK(memcmp(&out, &backup, sizeof out) == 0); /* atomicity */
    }
    rng = (Rng){0x89abcdefu, 0u, 0u};
    CHECK(warship_fleet_generate(&out, seeded, &rng) == WARSHIP_FLEET_OK);
    CHECK(rng.requests == 5u);
    verify_contents(&out);
    CHECK(warship_fleet_copy(&other, &out) == WARSHIP_FLEET_OK);
    CHECK(memcmp(&out, &other, sizeof out) == 0);
}

int main(void)
{
    geometry_tests();
    copying_tests();
    auto_tests();
    puts("warship_fleet_contract PASS");
    return EXIT_SUCCESS;
}
