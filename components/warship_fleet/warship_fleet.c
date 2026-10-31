#include "warship_fleet.h"

#include <string.h>

static const uint8_t lengths[WARSHIP_FLEET_SHIPS] = {2u, 3u, 3u, 4u, 5u};

uint8_t warship_fleet_ship_length(uint8_t id)
{
    return id < WARSHIP_FLEET_SHIPS ? lengths[id] : 0u;
}

static int fits(const WarshipShip *ship)
{
    if (ship->x >= WARSHIP_FLEET_GRID || ship->y >= WARSHIP_FLEET_GRID)
        return 0;
    if (ship->orientation == WARSHIP_ORIENT_HORIZONTAL)
        return (unsigned)ship->x + ship->length <= WARSHIP_FLEET_GRID;
    return (unsigned)ship->y + ship->length <= WARSHIP_FLEET_GRID;
}

static unsigned cell_index(const WarshipShip *ship, unsigned offset)
{
    const unsigned x = (unsigned)ship->x +
        (ship->orientation == WARSHIP_ORIENT_HORIZONTAL ? offset : 0u);
    const unsigned y = (unsigned)ship->y +
        (ship->orientation == WARSHIP_ORIENT_VERTICAL ? offset : 0u);
    return y * WARSHIP_FLEET_GRID + x;
}

WarshipFleetStatus warship_fleet_validate(const WarshipFleet *fleet)
{
    uint8_t occupied[WARSHIP_FLEET_GRID * WARSHIP_FLEET_GRID] = {0};
    unsigned ids = 0u;
    if (fleet == NULL) return WARSHIP_FLEET_INVALID_ARGUMENT;
    for (unsigned i = 0u; i < WARSHIP_FLEET_SHIPS; ++i) {
        const WarshipShip *ship = &fleet->ships[i];
        if (ship->id >= WARSHIP_FLEET_SHIPS) return WARSHIP_FLEET_INVALID_ID;
        const unsigned bit = 1u << ship->id;
        if ((ids & bit) != 0u) return WARSHIP_FLEET_DUPLICATE_ID;
        ids |= bit;
        if (ship->length != warship_fleet_ship_length(ship->id))
            return WARSHIP_FLEET_INVALID_LENGTH;
        if (ship->orientation != WARSHIP_ORIENT_HORIZONTAL &&
            ship->orientation != WARSHIP_ORIENT_VERTICAL)
            return WARSHIP_FLEET_INVALID_ORIENTATION;
        if (!fits(ship)) return WARSHIP_FLEET_OUT_OF_BOUNDS;
        for (unsigned k = 0u; k < ship->length; ++k) {
            const unsigned cell = cell_index(ship, k);
            if (occupied[cell] != 0u) return WARSHIP_FLEET_OVERLAP;
            occupied[cell] = 1u;
        }
    }
    return ids == (1u << WARSHIP_FLEET_SHIPS) - 1u ?
        WARSHIP_FLEET_OK : WARSHIP_FLEET_INVALID_ID;
}

WarshipFleetStatus warship_fleet_copy(WarshipFleet *destination,
                                      const WarshipFleet *source)
{
    if (destination == NULL || source == NULL)
        return WARSHIP_FLEET_INVALID_ARGUMENT;
    const WarshipFleetStatus status = warship_fleet_validate(source);
    if (status == WARSHIP_FLEET_OK) *destination = *source;
    return status;
}

static int free_cells(const uint8_t *occupied, const WarshipShip *ship)
{
    if (!fits(ship)) return 0;
    for (unsigned k = 0u; k < ship->length; ++k) {
        if (occupied[cell_index(ship, k)] != 0u) return 0;
    }
    return 1;
}

static void occupy(uint8_t *occupied, const WarshipShip *ship)
{
    for (unsigned k = 0u; k < ship->length; ++k)
        occupied[cell_index(ship, k)] = 1u;
}

WarshipFleetStatus warship_fleet_generate(WarshipFleet *destination,
                                          WarshipFleetRandom random_u32,
                                          void *random_context)
{
    WarshipFleet candidate = {{{0}}};
    uint8_t occupied[WARSHIP_FLEET_GRID * WARSHIP_FLEET_GRID] = {0};
    if (destination == NULL || random_u32 == NULL)
        return WARSHIP_FLEET_INVALID_ARGUMENT;

    for (unsigned id = 0u; id < WARSHIP_FLEET_SHIPS; ++id) {
        uint32_t sample = 0u;
        if (random_u32(random_context, &sample) != 0)
            return WARSHIP_FLEET_RANDOM_FAILURE;
        const unsigned start = sample % (WARSHIP_FLEET_GRID *
                                         WARSHIP_FLEET_GRID * 2u);
        int found = 0;
        for (unsigned step = 0u; step < WARSHIP_FLEET_GRID *
                                         WARSHIP_FLEET_GRID * 2u; ++step) {
            const unsigned index = (start + step) % (WARSHIP_FLEET_GRID *
                                                      WARSHIP_FLEET_GRID * 2u);
            WarshipShip ship = {
                (uint8_t)id,
                (uint8_t)((index / 2u) % WARSHIP_FLEET_GRID),
                (uint8_t)((index / 2u) / WARSHIP_FLEET_GRID),
                lengths[id],
                (uint8_t)(index % 2u)
            };
            if (free_cells(occupied, &ship)) {
                candidate.ships[id] = ship;
                occupy(occupied, &ship);
                found = 1;
                break;
            }
        }
        /* At most 12 occupied cells before any placement. The board has
         * 20 disjoint 5-cell horizontal blocks, hence a free candidate.
         * Returning an error is defensive; normal RNG values cannot reach it.
         */
        if (!found) return WARSHIP_FLEET_INTERNAL_ERROR;
    }
    if (warship_fleet_validate(&candidate) != WARSHIP_FLEET_OK)
        return WARSHIP_FLEET_INTERNAL_ERROR;
    *destination = candidate;
    return WARSHIP_FLEET_OK;
}
