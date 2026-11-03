#ifndef WARSHIP_DEPLOY_UI_H
#define WARSHIP_DEPLOY_UI_H

#include <stdint.h>
#include "warship_lifecycle.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WARSHIP_DEPLOY_W 320u
#define WARSHIP_DEPLOY_H 240u

typedef enum {
    WARSHIP_DEPLOY_NO_ACTION = 0,
    WARSHIP_DEPLOY_START,
    WARSHIP_DEPLOY_AUTO,
    WARSHIP_DEPLOY_READY,
    WARSHIP_DEPLOY_RETURN
} WarshipDeployAction;

typedef enum {
    WARSHIP_DEPLOY_OK = 0,
    WARSHIP_DEPLOY_BAD_ARGUMENT,
    WARSHIP_DEPLOY_BAD_LAYOUT,
    WARSHIP_DEPLOY_INVALID_FLEET,
    WARSHIP_DEPLOY_GAME_ERROR
} WarshipDeployStatus;

typedef struct {
    uint16_t x, y, w, h;
} WarshipDeployRect;

typedef struct {
    WarshipDeployRect grid;
    uint8_t cell;
    WarshipDeployRect auto_button, ready_button, return_button, start_button;
} WarshipDeployLayout;

/* No LCD/RTOS dependency. One gesture produces at most one command;
 * 40 ms stable down, 60 ms stable up, >=120 ms between accepted
 * actions. Input must use monotonic milliseconds and be sampled often.
 * UI dispatch and geometry can be host tested on real D07 lifecycle.
 */
typedef struct {
    WarshipDeployLayout layout;
    uint64_t since_ms;
    uint64_t last_action_ms;
    uint16_t first_x, first_y;
    uint8_t raw_down, armed, consumed, first_valid, has_action;
} WarshipDeployInput;

/* Public rendering value: no secret captain fleet or WarshipLifecycle.
 * 0=water; 1..5 are five different human ship identities.
 * mode 0=HOME, 1=DEPLOY, 2=DEMO_STOP_AFTER_READY.
 */
typedef struct {
    uint8_t cells[100];
    uint8_t mode;
} WarshipDeployFrame;

WarshipDeployLayout warship_deploy_default_layout(void);
int warship_deploy_layout_valid(const WarshipDeployLayout *layout);
WarshipDeployAction warship_deploy_hit(const WarshipDeployLayout *layout,
                                       uint8_t phase, uint16_t x, uint16_t y);
WarshipDeployStatus warship_deploy_input_init(WarshipDeployInput *input,
                                              const WarshipDeployLayout *layout);
/* pressed is PENIRQ LOW; valid=0 means SPI/mapping failure and the
 * WHOLE gesture is suppressed. The first DOWN captures the only candidate
 * point and disarms immediately; a short release/re-press cannot clear an
 * invalid sample or create a new gesture. Stable release >=60 ms rearms.
 * An interrupted DOWN (even initially valid) is not actionable after
 * bounce. Output NO_ACTION on polling, bounce, hold, and release.
 * Invalid timestamps or pointers do not mutate state/output.
 */
/* On returning from combat/result, require a full 60ms release before
 * any HOME/DEPLOY control can be activated (no inherited held tap). */
void warship_deploy_input_discard(WarshipDeployInput *input,
                                  uint64_t now_ms);
WarshipDeployStatus warship_deploy_input_step(
    WarshipDeployInput *input, uint8_t pressed, uint8_t valid,
    uint16_t x, uint16_t y, uint64_t now_ms, uint8_t phase,
    WarshipDeployAction *out);

/* Explicit D07 mutations, never invokes D07 fire/abandon/NVS. START
 * requires HOME; AUTO/READY/RETURN require DEPLOY. On failure D07
 * preserves the game. No "done" backdoor from the technical COMBAT stop.
 */
WarshipDeployStatus warship_deploy_apply(
    WarshipLifecycle *game, WarshipDeployAction action,
    WarshipFleetRandom human_rng, void *human_ctx,
    WarshipFleetRandom captain_rng, void *captain_ctx);

/* Frame is prepared from PUBLIC D07 projection ONLY. Reject invalid
 * fleets and bad phases without modifying output. Raster must never
 * access WarshipLifecycle or any captain placement.
 */
WarshipDeployStatus warship_deploy_make_frame(
    const WarshipLifecyclePublic *public_state, WarshipDeployFrame *out);
uint16_t warship_deploy_pixel(const WarshipDeployFrame *frame,
                              const WarshipDeployLayout *layout,
                              uint16_t x, uint16_t y);

#ifdef __cplusplus
}
#endif
#endif
