#ifndef WARSHIP_PALMARES_H
#define WARSHIP_PALMARES_H

#include <stddef.h>
#include <stdint.h>
#include "warship_lifecycle.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Single versioned, little-endian snapshot (including checksum).
 * One key/commit contains counters AND last applied event identity/type.
 */
#define WARSHIP_PALMARES_WIRE_SIZE 44u

typedef enum {
    WARSHIP_PALMARES_APPLIED = 0,
    WARSHIP_PALMARES_ALREADY_APPLIED,
    WARSHIP_PALMARES_NO_CHANGE,
    WARSHIP_PALMARES_BAD_ARGUMENT,
    WARSHIP_PALMARES_BAD_EVENT,
    WARSHIP_PALMARES_STALE_ID,
    WARSHIP_PALMARES_CONFLICTING_ID,
    WARSHIP_PALMARES_OVERFLOW,
    WARSHIP_PALMARES_READ_ERROR,
    WARSHIP_PALMARES_WRITE_UNCERTAIN,
    WARSHIP_PALMARES_RELOAD_REQUIRED,
    WARSHIP_PALMARES_CORRUPT,
    WARSHIP_PALMARES_INCOMPATIBLE_VERSION
} WarshipPalmaresStatus;

/* Load return: 0=found, 1=absent, other=IO error. On success *actual
 * is the full blob length; never copy more than capacity. An oversized
 * record reports its actual size without writing outside buffer.
 * Store return: 0=committed, nonzero=FAILED OR COMMIT UNCERTAIN.
 * Callbacks never modify ledger, output view, or backend configuration.
 */
typedef int (*WarshipPalmaresLoad)(void *ctx, uint8_t *data,
                                   size_t capacity, size_t *actual);
typedef int (*WarshipPalmaresStore)(void *ctx, const uint8_t *data,
                                    size_t size);
typedef struct {
    WarshipPalmaresLoad load;
    WarshipPalmaresStore store;
} WarshipPalmaresBackend;

/* Only these five counters are for PALMARÈS UI. No fleet or gameplay data. */
typedef struct {
    uint32_t games;
    uint32_t victories;
    uint32_t defeats;
    uint32_t draws;
    uint32_t voluntary_forfeits;
} WarshipPalmaresView;

/* Private ledger. Caller owns storage and backend context for its lifetime.
 * One writer only; no cross-task locking or automatic lifetime management.
 */
typedef struct {
    WarshipPalmaresView counters;
    WarshipPalmaresBackend backend;
    void *backend_context;
    uint64_t last_id;
    uint32_t last_event;
    uint32_t magic;
    uint8_t uncertain;
} WarshipPalmares;

/* Load persistent state, or zero snapshot for genuinely ABSENT storage.
 * Corrupt, truncated, oversize and incompatible versions are errors:
 * no erase, migration, silent reset or overwrite. Failure preserves out.
 */
WarshipPalmaresStatus warship_palmares_open(
    WarshipPalmares *out, WarshipPalmaresBackend backend, void *context);

/* Read cached validated counters; no storage operation, never increments. */
WarshipPalmaresStatus warship_palmares_read(
    const WarshipPalmares *ledger, WarshipPalmaresView *out);

/* Read the last DURABLY acknowledged (ID, type). Caller must not
 * inspect private internals or assume a CPU fleet is available.
 * Failure and uncertain ledger preserve both output parameters.
 */
WarshipPalmaresStatus warship_palmares_last_identity(
    const WarshipPalmares *ledger, uint64_t *id,
    WarshipLifecycleEvent *event);

/* Event identity MUST be generated outside D07, stable across retries,
 * strictly increasing on this CYD across ALL categories and restarts.
 * IDs start at 1; 0 is invalid for non-NONE events. Only the latest event
 * can return ALREADY_APPLIED; older IDs are STALE (never applied).
 * Same latest ID with a different type is CONFLICTING_ID.
 * NONE does not write regardless of the provided ID.
 * On store error, outcome is UNCERTAIN, ledger becomes reload-required
 * and subsequent applications are prohibited until successful reload.
 */
WarshipPalmaresStatus warship_palmares_apply(
    WarshipPalmares *ledger, uint64_t event_id, WarshipLifecycleEvent event);

/* Re-read source of truth after a commit error/uncertain response. If it
 * contains the attempted event, repeat apply yields ALREADY_APPLIED.
 * If it does not, the same event_id may be retried. On failure all
 * in-memory counters and uncertainty flag remain unchanged.
 */
WarshipPalmaresStatus warship_palmares_reload(WarshipPalmares *ledger);

#ifdef __cplusplus
}
#endif
#endif
