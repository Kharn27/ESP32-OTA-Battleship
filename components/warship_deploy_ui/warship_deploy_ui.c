#include "warship_deploy_ui.h"
#include <string.h>

static int inside_screen(WarshipDeployRect r)
{
    return r.w >= 8u && r.h >= 8u && r.x < WARSHIP_DEPLOY_W &&
           r.y < WARSHIP_DEPLOY_H &&
           (uint32_t)r.x + r.w <= WARSHIP_DEPLOY_W &&
           (uint32_t)r.y + r.h <= WARSHIP_DEPLOY_H;
}
static int overlap(WarshipDeployRect a, WarshipDeployRect b)
{
    return a.x < (uint32_t)b.x+b.w && b.x < (uint32_t)a.x+a.w &&
           a.y < (uint32_t)b.y+b.h && b.y < (uint32_t)a.y+a.h;
}
static int contains(WarshipDeployRect r, uint16_t x, uint16_t y)
{
    /* The outer 2-pixel rim of each button is NOT interactive. */
    return x >= (uint32_t)r.x+2u && y >= (uint32_t)r.y+2u &&
           (uint32_t)x+2u < (uint32_t)r.x+r.w &&
           (uint32_t)y+2u < (uint32_t)r.y+r.h;
}

WarshipDeployLayout warship_deploy_default_layout(void)
{
    const WarshipDeployLayout l = {
        .grid = {23u, 24u, 190u, 190u},
        .cell = 19u,
        .auto_button = {231u, 39u, 82u, 40u},
        .ready_button = {231u, 99u, 82u, 40u},
        .return_button = {231u, 159u, 82u, 40u},
        .start_button = {231u, 99u, 82u, 40u},
    };
    return l;
}
int warship_deploy_layout_valid(const WarshipDeployLayout *l)
{
    if (l == NULL || l->cell < 8u ||
        !inside_screen(l->grid) || !inside_screen(l->auto_button) ||
        !inside_screen(l->ready_button) || !inside_screen(l->return_button) ||
        !inside_screen(l->start_button) ||
        (unsigned)l->cell*10u != l->grid.w ||
        (unsigned)l->cell*10u != l->grid.h ||
        overlap(l->grid,l->auto_button) ||
        overlap(l->grid,l->ready_button) ||
        overlap(l->grid,l->return_button) ||
        overlap(l->auto_button,l->ready_button) ||
        overlap(l->auto_button,l->return_button) ||
        overlap(l->ready_button,l->return_button))
        return 0;
    return 1;
}
WarshipDeployAction warship_deploy_hit(const WarshipDeployLayout *l,
                                       uint8_t phase, uint16_t x, uint16_t y)
{
    if (!warship_deploy_layout_valid(l) ||
        x >= WARSHIP_DEPLOY_W || y >= WARSHIP_DEPLOY_H)
        return WARSHIP_DEPLOY_NO_ACTION;
    if (phase == WARSHIP_LIFECYCLE_HOME)
        return contains(l->start_button,x,y) ?
            WARSHIP_DEPLOY_START : WARSHIP_DEPLOY_NO_ACTION;
    if (phase != WARSHIP_LIFECYCLE_DEPLOY)
        return WARSHIP_DEPLOY_NO_ACTION;
    if (contains(l->auto_button,x,y)) return WARSHIP_DEPLOY_AUTO;
    if (contains(l->ready_button,x,y)) return WARSHIP_DEPLOY_READY;
    if (contains(l->return_button,x,y)) return WARSHIP_DEPLOY_RETURN;
    return WARSHIP_DEPLOY_NO_ACTION;
}
WarshipDeployStatus warship_deploy_input_init(WarshipDeployInput *in,
                                              const WarshipDeployLayout *l)
{
    if (in == NULL) return WARSHIP_DEPLOY_BAD_ARGUMENT;
    if (!warship_deploy_layout_valid(l)) return WARSHIP_DEPLOY_BAD_LAYOUT;
    memset(in,0,sizeof *in);
    in->layout = *l;
    in->armed = 1u;
    return WARSHIP_DEPLOY_OK;
}
WarshipDeployStatus warship_deploy_input_step(
    WarshipDeployInput *in, uint8_t pressed, uint8_t valid,
    uint16_t x, uint16_t y, uint64_t now_ms, uint8_t phase,
    WarshipDeployAction *out)
{
    if (in == NULL || out == NULL || pressed > 1u || valid > 1u ||
        !warship_deploy_layout_valid(&in->layout) ||
        (in->has_action && now_ms < in->last_action_ms) ||
        (in->raw_down && now_ms < in->since_ms) ||
        (in->has_action && now_ms - in->last_action_ms > UINT64_MAX/2u))
        return WARSHIP_DEPLOY_BAD_ARGUMENT;
    WarshipDeployAction action = WARSHIP_DEPLOY_NO_ACTION;
    if (pressed != in->raw_down) {
        in->raw_down = pressed;
        in->since_ms = now_ms;
        if (pressed) {
            in->first_x = x;
            in->first_y = y;
            in->first_valid = valid;
        }
    }
    if (pressed) {
        if (!valid) in->first_valid = 0u;
        if (in->armed && !in->consumed && in->first_valid &&
            now_ms - in->since_ms >= 40u &&
            (!in->has_action || now_ms-in->last_action_ms >= 120u)) {
            action = warship_deploy_hit(&in->layout, phase,
                                       in->first_x, in->first_y);
            in->consumed = 1u;
            in->armed = 0u;
            if (action != WARSHIP_DEPLOY_NO_ACTION) {
                in->last_action_ms = now_ms;
                in->has_action = 1u;
            }
        }
    } else if (now_ms - in->since_ms >= 60u) {
        in->armed = 1u;
        in->consumed = 0u;
    }
    *out = action;
    return WARSHIP_DEPLOY_OK;
}
WarshipDeployStatus warship_deploy_apply(
    WarshipLifecycle *game, WarshipDeployAction action,
    WarshipFleetRandom human_rng, void *human_ctx,
    WarshipFleetRandom captain_rng, void *captain_ctx)
{
    if (game == NULL) return WARSHIP_DEPLOY_BAD_ARGUMENT;
    WarshipLifecycleStatus result;
    switch (action) {
    case WARSHIP_DEPLOY_START:
        if (human_rng == NULL || captain_rng == NULL)
            return WARSHIP_DEPLOY_BAD_ARGUMENT;
        result = warship_lifecycle_start(game,human_rng,human_ctx,
                                         captain_rng,captain_ctx);
        break;
    case WARSHIP_DEPLOY_AUTO:
        if (human_rng == NULL) return WARSHIP_DEPLOY_BAD_ARGUMENT;
        result = warship_lifecycle_auto(game,human_rng,human_ctx); break;
    case WARSHIP_DEPLOY_READY:
        result = warship_lifecycle_ready(game); break;
    case WARSHIP_DEPLOY_RETURN:
        result = warship_lifecycle_leave_deploy(game); break;
    default: return WARSHIP_DEPLOY_BAD_ARGUMENT;
    }
    return result == WARSHIP_LIFECYCLE_OK ?
        WARSHIP_DEPLOY_OK : WARSHIP_DEPLOY_GAME_ERROR;
}
WarshipDeployStatus warship_deploy_make_frame(
    const WarshipLifecyclePublic *public_state, WarshipDeployFrame *out)
{
    if (public_state == NULL || out == NULL)
        return WARSHIP_DEPLOY_BAD_ARGUMENT;
    WarshipDeployFrame frame;
    memset(&frame,0,sizeof frame);
    if (public_state->phase == WARSHIP_LIFECYCLE_HOME) {
        if (public_state->has_human_fleet || public_state->has_battle)
            return WARSHIP_DEPLOY_INVALID_FLEET;
        frame.mode = 0u;
    } else if (public_state->phase == WARSHIP_LIFECYCLE_DEPLOY ||
               public_state->phase == WARSHIP_LIFECYCLE_COMBAT) {
        if (!public_state->has_human_fleet ||
            warship_fleet_validate(&public_state->human) != WARSHIP_FLEET_OK)
            return WARSHIP_DEPLOY_INVALID_FLEET;
        if ((public_state->phase == WARSHIP_LIFECYCLE_DEPLOY &&
             public_state->has_battle) ||
            (public_state->phase == WARSHIP_LIFECYCLE_COMBAT &&
             !public_state->has_battle))
            return WARSHIP_DEPLOY_INVALID_FLEET;
        frame.mode = public_state->phase == WARSHIP_LIFECYCLE_DEPLOY ? 1u : 2u;
        unsigned count = 0u;
        for (unsigned i=0u; i<WARSHIP_FLEET_SHIPS; ++i) {
            const WarshipShip *s = &public_state->human.ships[i];
            for (unsigned j=0u; j<s->length; ++j) {
                const unsigned x = (unsigned)s->x +
                    (s->orientation == WARSHIP_ORIENT_HORIZONTAL ? j : 0u);
                const unsigned y = (unsigned)s->y +
                    (s->orientation == WARSHIP_ORIENT_VERTICAL ? j : 0u);
                const unsigned index = y*WARSHIP_FLEET_GRID + x;
                if (index >= 100u || frame.cells[index] != 0u)
                    return WARSHIP_DEPLOY_INVALID_FLEET;
                frame.cells[index] = (uint8_t)(s->id+1u);
                ++count;
            }
        }
        if (count != 17u) return WARSHIP_DEPLOY_INVALID_FLEET;
    } else return WARSHIP_DEPLOY_BAD_ARGUMENT;
    *out = frame;
    return WARSHIP_DEPLOY_OK;
}
