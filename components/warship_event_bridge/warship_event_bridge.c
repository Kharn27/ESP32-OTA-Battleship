#include "warship_event_bridge.h"
#include <string.h>

static WarshipEventStatus map_ledger(WarshipPalmaresStatus s)
{
    switch (s) {
    case WARSHIP_PALMARES_APPLIED: return WARSHIP_EVENT_APPLIED;
    case WARSHIP_PALMARES_ALREADY_APPLIED: return WARSHIP_EVENT_ALREADY_APPLIED;
    case WARSHIP_PALMARES_NO_CHANGE: return WARSHIP_EVENT_NO_CHANGE;
    case WARSHIP_PALMARES_RELOAD_REQUIRED: return WARSHIP_EVENT_RELOAD_REQUIRED;
    case WARSHIP_PALMARES_WRITE_UNCERTAIN: return WARSHIP_EVENT_WRITE_UNCERTAIN;
    case WARSHIP_PALMARES_READ_ERROR: return WARSHIP_EVENT_READ_ERROR;
    case WARSHIP_PALMARES_CORRUPT: return WARSHIP_EVENT_CORRUPT;
    case WARSHIP_PALMARES_INCOMPATIBLE_VERSION:
        return WARSHIP_EVENT_INCOMPATIBLE_VERSION;
    case WARSHIP_PALMARES_OVERFLOW: return WARSHIP_EVENT_OVERFLOW;
    case WARSHIP_PALMARES_STALE_ID:
    case WARSHIP_PALMARES_CONFLICTING_ID: return WARSHIP_EVENT_CONFLICT;
    case WARSHIP_PALMARES_BAD_EVENT: return WARSHIP_EVENT_BAD_EVENT;
    default: return WARSHIP_EVENT_BAD_ARGUMENT;
    }
}
static WarshipEventStatus map_id_load(int rc)
{
    if (rc == -2) return WARSHIP_EVENT_CORRUPT;
    if (rc == -3) return WARSHIP_EVENT_INCOMPATIBLE_VERSION;
    return WARSHIP_EVENT_READ_ERROR;
}
static int valid_event(WarshipLifecycleEvent event)
{
    return event >= WARSHIP_LIFECYCLE_EVENT_VICTORY &&
           event <= WARSHIP_LIFECYCLE_EVENT_VOLUNTARY_FORFEIT;
}

WarshipEventStatus warship_event_bridge_open(
    WarshipEventBridge *out, WarshipPalmaresBackend ledger_backend,
    void *ledger_context, WarshipEventIdBackend identity_backend,
    void *identity_context)
{
    if (out == NULL || ledger_backend.load == NULL ||
        ledger_backend.store == NULL || identity_backend.load == NULL ||
        identity_backend.reserve == NULL)
        return WARSHIP_EVENT_BAD_ARGUMENT;
    WarshipEventBridge candidate;
    memset(&candidate, 0, sizeof candidate);
    const WarshipEventStatus opened = map_ledger(warship_palmares_open(
        &candidate.ledger, ledger_backend, ledger_context));
    if (opened != WARSHIP_EVENT_APPLIED) return opened;
    uint64_t ledger_id = 0u;
    WarshipLifecycleEvent last_event = WARSHIP_LIFECYCLE_EVENT_NONE;
    const WarshipEventStatus identity = map_ledger(warship_palmares_last_identity(
        &candidate.ledger, &ledger_id, &last_event));
    if (identity != WARSHIP_EVENT_APPLIED) return identity;
    uint64_t reservation_id = 0u;
    const int rc = identity_backend.load(identity_context, &reservation_id);
    if (rc != 0 && rc != 1) return map_id_load(rc);
    if (rc == 0 && (reservation_id == 0u || reservation_id < ledger_id))
        return WARSHIP_EVENT_CORRUPT;
    candidate.reserved_id =
        rc == 0 && reservation_id > ledger_id ? reservation_id : ledger_id;
    candidate.identity = identity_backend;
    candidate.identity_context = identity_context;
    candidate.opened = 1u;
    *out = candidate;
    return WARSHIP_EVENT_APPLIED;
}

static WarshipEventStatus attempt(WarshipEventBridge *bridge)
{
    if (!bridge->id_confirmed) {
        if (bridge->reservation_uncertain) return WARSHIP_EVENT_RELOAD_REQUIRED;
        if (bridge->identity.reserve(bridge->identity_context,
                                      bridge->pending_id) != 0) {
            bridge->reservation_uncertain = 1u;
            return WARSHIP_EVENT_WRITE_UNCERTAIN;
        }
        bridge->reserved_id = bridge->pending_id;
        bridge->id_confirmed = 1u;
    }
    const WarshipEventStatus st = map_ledger(warship_palmares_apply(
        &bridge->ledger, bridge->pending_id, bridge->pending_event));
    if (st == WARSHIP_EVENT_APPLIED || st == WARSHIP_EVENT_ALREADY_APPLIED) {
        bridge->pending = 0u;
        bridge->pending_id = 0u;
        bridge->pending_event = WARSHIP_LIFECYCLE_EVENT_NONE;
        bridge->id_confirmed = 0u;
        bridge->reservation_uncertain = 0u;
    }
    return st;
}

WarshipEventStatus warship_event_bridge_receive(
    WarshipEventBridge *bridge, WarshipLifecycleEvent event)
{
    if (bridge == NULL || !bridge->opened)
        return WARSHIP_EVENT_BAD_ARGUMENT;
    if (event == WARSHIP_LIFECYCLE_EVENT_NONE)
        return WARSHIP_EVENT_NO_CHANGE;
    if (!valid_event(event)) return WARSHIP_EVENT_BAD_EVENT;
    if (bridge->pending) return WARSHIP_EVENT_PENDING;
    if (bridge->reserved_id == UINT64_MAX)
        return WARSHIP_EVENT_OVERFLOW;
    bridge->pending_id = bridge->reserved_id + UINT64_C(1);
    bridge->pending_event = event;
    bridge->pending = 1u;
    bridge->id_confirmed = 0u;
    bridge->reservation_uncertain = 0u;
    return attempt(bridge);
}

WarshipEventStatus warship_event_bridge_retry(WarshipEventBridge *bridge)
{
    if (bridge == NULL || !bridge->opened)
        return WARSHIP_EVENT_BAD_ARGUMENT;
    if (!bridge->pending) return WARSHIP_EVENT_NO_CHANGE;
    if (bridge->reservation_uncertain) {
        uint64_t disk_id = 0u;
        const int rc = bridge->identity.load(bridge->identity_context,
                                              &disk_id);
        if (rc != 0 && rc != 1) return map_id_load(rc);
        /* Missing key is legal only for the first migration from a D08
         * ledger written before the reservation key existed. */
        if (rc == 1) {
            uint64_t ledger_id = 0u;
            WarshipLifecycleEvent last = WARSHIP_LIFECYCLE_EVENT_NONE;
            if (warship_palmares_last_identity(&bridge->ledger,
                                                &ledger_id, &last) !=
                                                WARSHIP_PALMARES_APPLIED ||
                bridge->reserved_id != ledger_id)
                return WARSHIP_EVENT_CORRUPT;
            disk_id = bridge->reserved_id;
        }
        if (disk_id == bridge->pending_id) {
            bridge->reserved_id = disk_id;
            bridge->id_confirmed = 1u;
        } else if (disk_id != bridge->reserved_id) {
            return WARSHIP_EVENT_CORRUPT;
        }
        bridge->reservation_uncertain = 0u;
    }
    /* D08 can remain uncertain independently of reservation. Reload
     * before ANY retry of the same ID/type; do not silently credit cache. */
    const WarshipPalmaresStatus r =
        warship_palmares_reload(&bridge->ledger);
    if (r != WARSHIP_PALMARES_APPLIED) return map_ledger(r);
    return attempt(bridge);
}

WarshipEventStatus warship_event_bridge_read(
    const WarshipEventBridge *bridge, WarshipPalmaresView *out)
{
    if (bridge == NULL || !bridge->opened || out == NULL)
        return WARSHIP_EVENT_BAD_ARGUMENT;
    return map_ledger(warship_palmares_read(&bridge->ledger, out));
}

WarshipEventStatus warship_event_bridge_last(
    const WarshipEventBridge *bridge, uint64_t *id,
    WarshipLifecycleEvent *event)
{
    if (bridge == NULL || !bridge->opened)
        return WARSHIP_EVENT_BAD_ARGUMENT;
    return map_ledger(warship_palmares_last_identity(&bridge->ledger,id,event));
}

int warship_event_bridge_has_pending(const WarshipEventBridge *bridge)
{
    return bridge != NULL && bridge->opened && bridge->pending;
}
