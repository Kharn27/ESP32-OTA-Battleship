#ifndef WARSHIP_FLEET_H
#define WARSHIP_FLEET_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WARSHIP_FLEET_GRID 10u
#define WARSHIP_FLEET_SHIPS 5u
#define WARSHIP_FLEET_CELLS 17u

typedef enum {
    WARSHIP_SHIP_PATROL = 0,
    WARSHIP_SHIP_SUBMARINE = 1,
    WARSHIP_SHIP_DESTROYER = 2,
    WARSHIP_SHIP_CRUISER = 3,
    WARSHIP_SHIP_CARRIER = 4
} WarshipShipId;

typedef enum {
    WARSHIP_ORIENT_HORIZONTAL = 0,
    WARSHIP_ORIENT_VERTICAL = 1
} WarshipOrientation;

/* x increases to the right, y increases downwards, origin (0,0).
 * The first occupied square is (x,y). length is checked against id.
 * Public struct fields are bytes; IDs 1 and 2 are DISTINCT 3-cell ships.
 */
typedef struct {
    uint8_t id;
    uint8_t x;
    uint8_t y;
    uint8_t length;
    uint8_t orientation;
} WarshipShip;

typedef struct {
    WarshipShip ships[WARSHIP_FLEET_SHIPS];
} WarshipFleet;

typedef enum {
    WARSHIP_FLEET_OK = 0,
    WARSHIP_FLEET_INVALID_ARGUMENT,
    WARSHIP_FLEET_INVALID_ID,
    WARSHIP_FLEET_DUPLICATE_ID,
    WARSHIP_FLEET_INVALID_LENGTH,
    WARSHIP_FLEET_INVALID_ORIENTATION,
    WARSHIP_FLEET_OUT_OF_BOUNDS,
    WARSHIP_FLEET_OVERLAP,
    WARSHIP_FLEET_RANDOM_FAILURE,
    WARSHIP_FLEET_INTERNAL_ERROR
} WarshipFleetStatus;

/* Return zero and write *value on success; nonzero on failure.
 * Callback must not modify the output fleet. No randomness quality,
 * independence or statistical uniformity is enforced by this component.
 */
typedef int (*WarshipFleetRandom)(void *context, uint32_t *value);

/* Return 0 for an unknown id. All five identities are distinct. */
uint8_t warship_fleet_ship_length(uint8_t id);

/* Strict validation of all five ships; touching by sides/corners is legal.
 * No ownership transfer, heap allocation or output side effects.
 */
WarshipFleetStatus warship_fleet_validate(const WarshipFleet *fleet);

/* On failure, *destination remains byte-for-byte unchanged. Aliasing
 * destination==source is supported. Both pointers must be valid.
 */
WarshipFleetStatus warship_fleet_copy(WarshipFleet *destination,
                                      const WarshipFleet *source);

/* One full fleet, for either player or Capitaine (the caller owns secrecy).
 * Exactly five RNG requests, <= 1000 candidate probes, no heap or retry
 * without a fixed upper bound. A free position always exists: the grid
 * contains 20 disjoint length-5 horizontal slots; before the last ship,
 * at most 12 cells are occupied, so at least one slot is empty.
 * On any failure, *destination is not changed. Identical results are legal.
 */
WarshipFleetStatus warship_fleet_generate(WarshipFleet *destination,
                                          WarshipFleetRandom random_u32,
                                          void *random_context);

#ifdef __cplusplus
}
#endif
#endif
