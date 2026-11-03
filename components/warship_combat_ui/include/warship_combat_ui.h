#ifndef WARSHIP_COMBAT_UI_H
#define WARSHIP_COMBAT_UI_H

#include <stdint.h>
#include "warship_lifecycle.h"

#ifdef __cplusplus
extern "C" {
#endif

/* D10 technical geometry, not G04/G05 or a PO-approved hitbox contract. */
#define WARSHIP_COMBAT_REVEAL_MS 180u
#define WARSHIP_COMBAT_ATTACK 0u
#define WARSHIP_COMBAT_DEFENSE 1u

typedef enum {
    WARSHIP_COMBAT_OK = 0,
    WARSHIP_COMBAT_IGNORED,
    WARSHIP_COMBAT_BAD_ARGUMENT,
    WARSHIP_COMBAT_GAME_ERROR
} WarshipCombatStatus;

typedef enum {
    WARSHIP_COMBAT_NO_HIT = 0,
    WARSHIP_COMBAT_CELL,
    WARSHIP_COMBAT_FIRE,
    WARSHIP_COMBAT_SWITCH,
    WARSHIP_COMBAT_CANCEL
} WarshipCombatCommand;

typedef struct {
    uint8_t command;
    uint8_t x, y; /* 0..9 only for CELL */
} WarshipCombatHit;

/* B01 gesture contract: 40ms DOWN, 60ms UP, 120ms between actions.
 * Initialized DISARMED after READY: the READY press cannot become FEU.
 * During reveal all contacts are consumed, never queued or replayed.
 */
typedef struct {
    uint64_t since_ms, last_ms, last_action_ms;
    uint16_t first_x, first_y;
    uint8_t raw_down, armed, consumed, first_valid, has_action;
} WarshipCombatInput;

/* Presentation controller: only public resolved pairs, no secret fleet.
 * The application alone owns the private WarshipLifecycle.
 */
typedef struct {
    WarshipBattlePair pending_pair;
    WarshipBattleShot last_cpu;
    uint64_t fire_ms;
    uint32_t reveals;
    uint8_t view;
    uint8_t pending;
    uint8_t has_summary;
    uint8_t alert;          /* 0 none, 1 MISS, 2 HIT, 3 SUNK */
    uint8_t terminal_event; /* D07 one-shot, diagnostic only, no D08 write */
} WarshipCombatUi;

/* Raster accepts ONLY this public render value (never lifecycle/session).
 * ships[] is populated ONLY for the human DEFENSE grid. In ATTAQUE it
 * is zero everywhere, even after a SUNK announcement.
 */
typedef struct {
    uint8_t attacks[100];  /* 0 unknown; 1 MISS; 2 HIT; 3 SUNK last cell */
    uint8_t ships[100];    /* defense: 0 water; 1..5 human identity */
    WarshipBattleShot last_cpu, last_human;
    uint8_t view, busy, has_selection, target_x, target_y;
    uint8_t has_cpu, has_human, alert, verdict, terminal;
    uint8_t rounds;
} WarshipCombatFrame;

void warship_combat_init(WarshipCombatUi *ui, WarshipCombatInput *in,
                        uint64_t now_ms);
WarshipCombatHit warship_combat_hit(uint8_t view, uint16_t x, uint16_t y);
/* Disarm for any press that began while synchronous LCD DMA blocked sampling. */
void warship_combat_input_discard(WarshipCombatInput *in);
WarshipCombatStatus warship_combat_input_step(
    WarshipCombatInput *in, uint8_t down, uint8_t valid,
    uint16_t x, uint16_t y, uint64_t now_ms, uint8_t blocked,
    uint8_t view, WarshipCombatHit *out);

/* Controller dispatcher; never pass a lifecycle to the raster.
 * FEU is a single atomic D07/D06/D02 transaction. No ACK here.
 */
WarshipCombatStatus warship_combat_apply(
    WarshipCombatUi *ui, WarshipLifecycle *game,
    const WarshipLifecyclePublic *observed, WarshipCombatHit hit,
    uint64_t now_ms, WarshipCaptainRandom rng, void *rng_ctx);
int warship_combat_reveal_due(const WarshipCombatUi *ui, uint64_t now_ms);
/* Snapshot hides unresolved pair; include_pending explicitly adds BOTH
 * completed results to one frame, for synchronous reveal BEFORE ACK.
 */
WarshipCombatStatus warship_combat_make_frame(
    const WarshipLifecyclePublic *public_state, const WarshipCombatUi *ui,
    uint8_t include_pending, WarshipCombatFrame *out);
/* Call ONLY after the complete pair frame was successfully sent to LCD.
 * Consumes D07 event once; it is logged, not persisted in D10.
 */
WarshipCombatStatus warship_combat_presented(
    WarshipCombatUi *ui, WarshipLifecycle *game,
    WarshipLifecycleEvent *event);
uint8_t warship_combat_max_alert(uint8_t old_level, uint8_t impact);
uint16_t warship_combat_pixel(const WarshipCombatFrame *frame,
                             uint16_t x, uint16_t y);

#ifdef __cplusplus
}
#endif
#endif
