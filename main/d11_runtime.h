#ifndef WARSHIP_D11_RUNTIME_H
#define WARSHIP_D11_RUNTIME_H

#include "warship_lifecycle.h"
#include "warship_event_bridge.h"

/* D11: one app_main owner, no player-facing UI/secret fleet access.
 * All NVS calls are synchronous and only at boot/post-ACK/periodic
 * non-SPI points. Not callable from an ISR or a DMA callback. */
void d11_runtime_init(void);
/* Return actual D11 status for D12: an uncertain write is NOT success. */
WarshipEventStatus d11_runtime_event(WarshipLifecycleEvent event);
void d11_runtime_poll(void);

#endif
