#include "warship_inspection.h"

#include <string.h>

static void wipe(void *p, size_t bytes)
{
    volatile unsigned char *v=(volatile unsigned char *)p;
    while(bytes--!=0u)*v++=0u;
}
void warship_inspection_clear_frame(WarshipCombatFrame *frame)
{
    if(frame!=NULL)wipe(frame,sizeof *frame);
}
WarshipInspectionStatus warship_inspection_toggle(
    WarshipLifecycle *game,const WarshipCombatUi *ui,WarshipCombatHit hit)
{
    if(game==NULL || ui==NULL || hit.command>WARSHIP_COMBAT_INSPECT)
        return WARSHIP_INSPECTION_BAD_ARGUMENT;
    if(hit.command!=WARSHIP_COMBAT_INSPECT)
        return WARSHIP_INSPECTION_NOT_HANDLED;
    if(ui->pending || ui->view!=WARSHIP_COMBAT_ATTACK)
        return WARSHIP_INSPECTION_IGNORED;
    WarshipLifecyclePublic view;
    if(warship_lifecycle_observe(game,&view)!=WARSHIP_LIFECYCLE_OK)
        return WARSHIP_INSPECTION_GAME_ERROR;
    if(view.phase!=WARSHIP_LIFECYCLE_RESULT || !view.has_battle ||
       !view.can_reveal_captain ||
       view.battle.history.verdict!=WARSHIP_BATTLE_DEFEAT)
        return WARSHIP_INSPECTION_IGNORED;
    const WarshipLifecycleStatus st=warship_lifecycle_set_captain_visible(
        game,(uint8_t)!view.reveal_visible);
    return st==WARSHIP_LIFECYCLE_OK?WARSHIP_INSPECTION_APPLIED:
           WARSHIP_INSPECTION_GAME_ERROR;
}
WarshipInspectionStatus warship_inspection_overlay(
    const WarshipLifecycle *game,const WarshipLifecyclePublic *observed,
    const WarshipCombatUi *ui,WarshipCombatFrame *frame)
{
    if(game==NULL || observed==NULL || ui==NULL || frame==NULL ||
       ui->view>WARSHIP_COMBAT_DEFENSE ||
       frame->view!=ui->view || frame->phase!=observed->phase ||
       frame->phase>WARSHIP_LIFECYCLE_RESULT)
        return WARSHIP_INSPECTION_BAD_ARGUMENT;
    /* Only D07 flags, never a CPU fleet. The base frame is already
     * assembled by D10 (and independently rejects invalid shots).
     * Fail CLOSED on contradictions; caller clears on error. */
    frame->reveal_available=0u;
    frame->reveal_visible=0u;
    frame->reveal_requested=0u;
    if(observed->phase!=WARSHIP_LIFECYCLE_RESULT) {
        if(observed->can_reveal_captain || observed->reveal_requested ||
           observed->reveal_visible)return WARSHIP_INSPECTION_BAD_ARGUMENT;
        return WARSHIP_INSPECTION_APPLIED;
    }
    if(observed->battle.history.verdict!=WARSHIP_BATTLE_DEFEAT) {
        if(observed->can_reveal_captain || observed->reveal_requested ||
           observed->reveal_visible)return WARSHIP_INSPECTION_BAD_ARGUMENT;
        return WARSHIP_INSPECTION_APPLIED;
    }
    if(!observed->can_reveal_captain ||
       (!observed->reveal_requested && observed->reveal_visible) ||
       ui->pending)
        return WARSHIP_INSPECTION_BAD_ARGUMENT;
    frame->reveal_available=1u;
    frame->reveal_requested=observed->reveal_requested;
    frame->reveal_visible=observed->reveal_visible;
    /* DÉFENSE always remains HUMAN only, regardless of visibility;
     * D07 keeps reveal_visible across navigation. */
    if(ui->view!=WARSHIP_COMBAT_ATTACK || !observed->reveal_visible)
        return WARSHIP_INSPECTION_APPLIED;
    WarshipFleet fleet;
    memset(&fleet,0,sizeof fleet);
    const WarshipLifecycleStatus s=
        warship_lifecycle_copy_revealed_captain(game,&fleet);
    if(s!=WARSHIP_LIFECYCLE_OK) {
        wipe(&fleet,sizeof fleet);
        return WARSHIP_INSPECTION_GAME_ERROR;
    }
    if(warship_fleet_validate(&fleet)!=WARSHIP_FLEET_OK) {
        wipe(&fleet,sizeof fleet);
        return WARSHIP_INSPECTION_GAME_ERROR;
    }
    /* No heap or second framebuffer; only ships[] is populated.
     * Attacks[] (all historical MISS/HIT/SUNK) remains unchanged. */
    for(unsigned i=0u;i<WARSHIP_FLEET_SHIPS;++i) {
        const WarshipShip *ship=&fleet.ships[i];
        for(unsigned j=0u;j<ship->length;++j) {
            const unsigned x=ship->x+
                (ship->orientation==WARSHIP_ORIENT_HORIZONTAL?j:0u);
            const unsigned y=ship->y+
                (ship->orientation==WARSHIP_ORIENT_VERTICAL?j:0u);
            const unsigned index=y*WARSHIP_FLEET_GRID+x;
            if(index>=WARSHIP_FLEET_GRID*WARSHIP_FLEET_GRID ||
               frame->ships[index]!=0u) {
                wipe(&fleet,sizeof fleet);
                return WARSHIP_INSPECTION_GAME_ERROR;
            }
            frame->ships[index]=(uint8_t)(ship->id+1u);
        }
    }
    wipe(&fleet,sizeof fleet);
    return WARSHIP_INSPECTION_APPLIED;
}
