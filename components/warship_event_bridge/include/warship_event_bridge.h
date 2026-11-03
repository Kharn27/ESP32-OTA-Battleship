#ifndef WARSHIP_EVENT_BRIDGE_H
#define WARSHIP_EVENT_BRIDGE_H

#include <stdint.h>
#include "warship_palmares.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 0=existing reservation, 1=absent, -1=I/O error,
 * -2=invalid/corrupt record, -3=incompatible version.
 * reserve() returns 0 only after a confirmed durable commit;
 * any other return is UNCERTAIN (the reservation may have persisted).
 * Single owner/writer. No event/fleet data is stored by the ID backend.
 */
typedef int (*WarshipEventIdLoad)(void *context, uint64_t *out);
typedef int (*WarshipEventIdReserve)(void *context, uint64_t id);
typedef struct {
    WarshipEventIdLoad load;
    WarshipEventIdReserve reserve;
} WarshipEventIdBackend;

typedef enum {
    WARSHIP_EVENT_PENDING = 0,
    WARSHIP_EVENT_APPLIED,
    WARSHIP_EVENT_ALREADY_APPLIED,
    WARSHIP_EVENT_NO_CHANGE,
    WARSHIP_EVENT_WRITE_UNCERTAIN,
    WARSHIP_EVENT_RELOAD_REQUIRED,
    WARSHIP_EVENT_READ_ERROR,
    WARSHIP_EVENT_CORRUPT,
    WARSHIP_EVENT_INCOMPATIBLE_VERSION,
    WARSHIP_EVENT_OVERFLOW,
    WARSHIP_EVENT_CONFLICT,
    WARSHIP_EVENT_BAD_EVENT,
    WARSHIP_EVENT_BAD_ARGUMENT
} WarshipEventStatus;

typedef struct {
    WarshipPalmares ledger;
    WarshipEventIdBackend identity;
    void *identity_context;
    uint64_t reserved_id;  /* confirmed high-water on flash, or old D08 last ID */
    uint64_t pending_id;   /* sticky across every same-process retry */
    WarshipLifecycleEvent pending_event;
    uint8_t pending;
    uint8_t id_confirmed;
    uint8_t reservation_uncertain;
    uint8_t opened;
} WarshipEventBridge;

/* Open reads D08 plus the durable ID high-water; NO boot-time write.
 * An older D08 ledger without an ID reservation key is supported.
 * Corrupt/incompatible records and an impossible sequence fail closed.
 */
WarshipEventStatus warship_event_bridge_open(
    WarshipEventBridge *out, WarshipPalmaresBackend ledger_backend,
    void *ledger_context, WarshipEventIdBackend identity_backend,
    void *identity_context);

/* Call ONLY after successful D07 terminal ACK or confirmed abandon.
 * NONE never writes. If a previous event is pending, a new event is
 * rejected without replacing its ID/type, even if the type is identical.
 * Reserve an ID durably BEFORE applying the D08 snapshot, permitting
 * sequence gaps across crashes without recycling an already reserved ID.
 */
WarshipEventStatus warship_event_bridge_receive(
    WarshipEventBridge *bridge, WarshipLifecycleEvent event);

/* Explicit bounded retry (one reservation+ledger attempt at most).
 * Reload is mandatory after an uncertain response; no automatic spin.
 */
WarshipEventStatus warship_event_bridge_retry(WarshipEventBridge *bridge);

/* Read the five public counters (no fleet), or cached durable identity.
 * Pending/uncertain status is reported separately; no invented credit.
 */
WarshipEventStatus warship_event_bridge_read(
    const WarshipEventBridge *bridge, WarshipPalmaresView *out);
WarshipEventStatus warship_event_bridge_last(
    const WarshipEventBridge *bridge, uint64_t *id,
    WarshipLifecycleEvent *event);
int warship_event_bridge_has_pending(const WarshipEventBridge *bridge);

#ifdef __cplusplus
}
#endif
#endif
