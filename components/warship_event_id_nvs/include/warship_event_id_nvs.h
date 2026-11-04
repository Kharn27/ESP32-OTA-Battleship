#ifndef WARSHIP_EVENT_ID_NVS_H
#define WARSHIP_EVENT_ID_NVS_H

#include "esp_err.h"
#include "warship_event_bridge.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Reservation-only metadata, no result/forfeit counters, no fleets.
 * In production it lives alongside D08 under warship/event_seq, while
 * the unchanged 44-byte D08 snapshot remains warship/palmares.
 * A NULL namespace_name defaults to warship, other names are only for
 * explicitly selected isolated diagnostic benches.
 */
typedef struct {
    esp_err_t last_error;
    const char *namespace_name;
} WarshipEventIdNvsContext;

WarshipEventIdBackend warship_event_id_nvs_backend(void);

#ifdef __cplusplus
}
#endif
#endif
