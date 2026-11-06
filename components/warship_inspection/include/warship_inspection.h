#ifndef WARSHIP_INSPECTION_H
#define WARSHIP_INSPECTION_H

#include "warship_combat_ui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WARSHIP_INSPECTION_APPLIED=0,
    WARSHIP_INSPECTION_NOT_HANDLED,
    WARSHIP_INSPECTION_IGNORED,
    WARSHIP_INSPECTION_BAD_ARGUMENT,
    WARSHIP_INSPECTION_GAME_ERROR
} WarshipInspectionStatus;

/* C99 controller, no NVS, no allocation, no TFT dependency. Only
 * an opt-in INSPECT gesture after RESULT DEFEAT in ATTAQUE may toggle.
 * 'observed' is the caller-owned, freshly refreshed D07 public projection
 * (D09 keeps it STATIC to prevent a large duplicate main-task stack frame).
 * No game->captain read: D07 independently enforces permission. */
WarshipInspectionStatus warship_inspection_toggle(
    WarshipLifecycle *game, const WarshipLifecyclePublic *observed,
    const WarshipCombatUi *ui, WarshipCombatHit hit);

/* Call after ordinary D10 public frame building, before drawing. The
 * controller may obtain ONE short-lived D07-gated WarshipFleet copy
 * during opt-in RESULT DEFEAT in ATTAQUE. In-place frame.ships[] only;
 * frame.attacks[] is never touched. No CPU content in WarshipLifecyclePublic,
 * any RTOS callback, a persisted score or any AI input. */
WarshipInspectionStatus warship_inspection_overlay(
    const WarshipLifecycle *game, const WarshipLifecyclePublic *observed,
    const WarshipCombatUi *ui, WarshipCombatFrame *frame);

/* Drop all secret-derived presentation pixels immediately on HIDE, EXIT,
 * REPLAY or a new session. The next display must rebuild from D07.
 * Clears public frame too; never call between LCD bands. */
void warship_inspection_clear_frame(WarshipCombatFrame *frame);

#ifdef __cplusplus
}
#endif
#endif
