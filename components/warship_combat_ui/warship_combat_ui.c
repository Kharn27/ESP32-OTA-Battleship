#include "warship_combat_ui.h"
#include <stddef.h>
#include <string.h>

/* Provisional D10 coordinates; 19-pixel cells, one 190x190 grid. */
#define GRID_X 22u
#define GRID_Y 24u
#define CELL 19u

static int in_button(unsigned x, unsigned y, unsigned bx, unsigned by)
{
    return x >= bx+2u && x < bx+89u && y >= by+2u && y < by+36u;
}

void warship_combat_init(WarshipCombatUi *ui, WarshipCombatInput *in,
                        uint64_t now_ms)
{
    if (ui != NULL) memset(ui, 0, sizeof *ui);
    if (in != NULL) {
        memset(in, 0, sizeof *in);
        in->since_ms = now_ms;
        in->last_ms = now_ms;
        in->raw_down = 1u; /* READY is still being held */
        in->consumed = 1u;
    }
}

WarshipCombatHit warship_combat_hit(uint8_t view, uint16_t x, uint16_t y)
{
    WarshipCombatHit hit = {WARSHIP_COMBAT_NO_HIT,0u,0u};
    if (view > WARSHIP_COMBAT_DEFENSE || x >= 320u || y >= 240u)
        return hit;
    if (in_button(x,y,225u,83u)) {
        hit.command = WARSHIP_COMBAT_SWITCH;
    } else if (view == WARSHIP_COMBAT_ATTACK) {
        if (x >= GRID_X && x < GRID_X+190u &&
            y >= GRID_Y && y < GRID_Y+190u) {
            hit.command = WARSHIP_COMBAT_CELL;
            hit.x = (uint8_t)((x-GRID_X)/CELL);
            hit.y = (uint8_t)((y-GRID_Y)/CELL);
        } else if (in_button(x,y,225u,32u))
            hit.command = WARSHIP_COMBAT_FIRE;
        else if (in_button(x,y,225u,134u))
            hit.command = WARSHIP_COMBAT_CANCEL;
    }
    return hit;
}

WarshipCombatStatus warship_combat_input_step(
    WarshipCombatInput *in, uint8_t down, uint8_t valid,
    uint16_t x, uint16_t y, uint64_t ms, uint8_t blocked,
    uint8_t view, WarshipCombatHit *out)
{
    if (in == NULL || out == NULL || down > 1u || valid > 1u ||
        blocked > 1u || view > WARSHIP_COMBAT_DEFENSE ||
        ms < in->last_ms)
        return WARSHIP_COMBAT_BAD_ARGUMENT;
    WarshipCombatHit hit = {WARSHIP_COMBAT_NO_HIT,0u,0u};
    if (down != in->raw_down) {
        in->raw_down = down;
        in->since_ms = ms;
        if (down && in->armed) {
            in->armed = 0u;
            in->consumed = 0u;
            in->first_x = x;
            in->first_y = y;
            in->first_valid = valid;
        } else if (!down) {
            in->consumed = 1u;
        }
    }
    if (blocked) {
        in->consumed = 1u; /* permanently discard this gesture */
        in->armed = 0u;
    }
    if (down) {
        if (!valid) in->first_valid = 0u;
        if (!blocked && !in->armed && !in->consumed && in->first_valid &&
            ms - in->since_ms >= 40u &&
            (!in->has_action || ms - in->last_action_ms >= 120u)) {
            hit = warship_combat_hit(view,in->first_x,in->first_y);
            in->consumed = 1u;
            if (hit.command != WARSHIP_COMBAT_NO_HIT) {
                in->has_action = 1u;
                in->last_action_ms = ms;
            }
        }
    } else if (ms - in->since_ms >= 60u) {
        in->armed = 1u;
        in->consumed = 0u;
        in->first_valid = 0u;
    }
    in->last_ms = ms;
    *out = hit;
    return WARSHIP_COMBAT_OK;
}

WarshipCombatStatus warship_combat_apply(
    WarshipCombatUi *ui, WarshipLifecycle *game, WarshipCombatHit hit,
    uint64_t ms, WarshipCaptainRandom rng, void *rng_ctx)
{
    if (ui == NULL || game == NULL || hit.command > WARSHIP_COMBAT_CANCEL)
        return WARSHIP_COMBAT_BAD_ARGUMENT;
    if (ui->pending || hit.command == WARSHIP_COMBAT_NO_HIT)
        return WARSHIP_COMBAT_IGNORED;
    /* RESULT is technically inspectable but cannot select or FIRE. */
    if (game->phase != WARSHIP_LIFECYCLE_COMBAT &&
        game->phase != WARSHIP_LIFECYCLE_RESULT)
        return WARSHIP_COMBAT_IGNORED;
    if (hit.command == WARSHIP_COMBAT_SWITCH) {
        ui->view = (uint8_t)(ui->view == WARSHIP_COMBAT_ATTACK ?
                            WARSHIP_COMBAT_DEFENSE : WARSHIP_COMBAT_ATTACK);
        if (ui->view == WARSHIP_COMBAT_DEFENSE) {
            ui->has_summary = 0u;
            ui->alert = 0u;
        }
        return WARSHIP_COMBAT_OK;
    }
    if (game->phase != WARSHIP_LIFECYCLE_COMBAT ||
        ui->view != WARSHIP_COMBAT_ATTACK)
        return WARSHIP_COMBAT_IGNORED;
    WarshipLifecyclePublic snapshot;
    if (warship_lifecycle_observe(game,&snapshot) != WARSHIP_LIFECYCLE_OK)
        return WARSHIP_COMBAT_GAME_ERROR;
    if (snapshot.battle.phase != WARSHIP_SOLO_SELECTING)
        return WARSHIP_COMBAT_IGNORED;
    WarshipLifecycleStatus s = WARSHIP_LIFECYCLE_BAD_ARGUMENT;
    if (hit.command == WARSHIP_COMBAT_CELL) {
        if (hit.x >= 10u || hit.y >= 10u)
            return WARSHIP_COMBAT_IGNORED;
        if (snapshot.battle.has_selection &&
            snapshot.battle.target_x == hit.x &&
            snapshot.battle.target_y == hit.y)
            s = warship_lifecycle_cancel(game);
        else
            s = warship_lifecycle_select(game,hit.x,hit.y);
    } else if (hit.command == WARSHIP_COMBAT_CANCEL) {
        if (!snapshot.battle.has_selection) return WARSHIP_COMBAT_IGNORED;
        s = warship_lifecycle_cancel(game);
    } else if (hit.command == WARSHIP_COMBAT_FIRE) {
        if (!snapshot.battle.has_selection || rng == NULL)
            return WARSHIP_COMBAT_IGNORED;
        WarshipBattlePair pair;
        s = warship_lifecycle_fire(game,rng,rng_ctx,&pair);
        if (s == WARSHIP_LIFECYCLE_OK) {
            ui->pending_pair = pair;
            ui->pending = 1u;
            ui->fire_ms = ms;
        }
    }
    if (s == WARSHIP_LIFECYCLE_ALREADY_ATTACKED ||
        s == WARSHIP_LIFECYCLE_NO_SELECTION ||
        s == WARSHIP_LIFECYCLE_DUPLICATE_SELECTION ||
        s == WARSHIP_LIFECYCLE_BAD_PHASE)
        return WARSHIP_COMBAT_IGNORED;
    return s == WARSHIP_LIFECYCLE_OK ? WARSHIP_COMBAT_OK :
           WARSHIP_COMBAT_GAME_ERROR;
}

int warship_combat_reveal_due(const WarshipCombatUi *ui, uint64_t ms)
{
    return ui != NULL && ui->pending && ms >= ui->fire_ms &&
           ms-ui->fire_ms >= WARSHIP_COMBAT_REVEAL_MS;
}

uint8_t warship_combat_max_alert(uint8_t old_level, uint8_t impact)
{
    if (old_level > 3u || impact > WARSHIP_BATTLE_SUNK) return old_level;
    const uint8_t incoming = (uint8_t)(impact+1u);
    return old_level > incoming ? old_level : incoming;
}

static int shot_ok(WarshipBattleShot shot)
{
    return shot.x < 10u && shot.y < 10u &&
           shot.impact <= WARSHIP_BATTLE_SUNK &&
           (shot.impact == WARSHIP_BATTLE_SUNK ?
             shot.ship_id < WARSHIP_FLEET_SHIPS :
             shot.ship_id == WARSHIP_BATTLE_NO_SHIP);
}

WarshipCombatStatus warship_combat_make_frame(
    const WarshipLifecyclePublic *p, const WarshipCombatUi *ui,
    uint8_t include_pending, WarshipCombatFrame *out)
{
    if (p == NULL || ui == NULL || out == NULL ||
        include_pending > 1u ||
        ui->view > WARSHIP_COMBAT_DEFENSE ||
        (include_pending && (!ui->pending || ui->view != WARSHIP_COMBAT_ATTACK)) ||
        (p->phase != WARSHIP_LIFECYCLE_COMBAT &&
         p->phase != WARSHIP_LIFECYCLE_RESULT) ||
        !p->has_battle || !p->has_human_fleet ||
        p->battle.history.outgoing_count > 100u ||
        p->battle.history.incoming_count > 100u ||
        warship_fleet_validate(&p->human) != WARSHIP_FLEET_OK)
        return WARSHIP_COMBAT_BAD_ARGUMENT;
    WarshipCombatFrame f;
    memset(&f,0,sizeof f);
    f.view = ui->view;
    f.busy = (uint8_t)(ui->pending && !include_pending);
    f.verdict = p->battle.history.verdict;
    f.terminal = (uint8_t)(p->phase == WARSHIP_LIFECYCLE_RESULT);
    f.rounds = p->battle.history.outgoing_count;
    if (f.view == WARSHIP_COMBAT_DEFENSE) {
        for (unsigned i=0u; i<WARSHIP_FLEET_SHIPS; ++i) {
            const WarshipShip *s = &p->human.ships[i];
            for (unsigned n=0u; n<s->length; ++n) {
                const unsigned xx=s->x+(s->orientation==WARSHIP_ORIENT_HORIZONTAL?n:0u);
                const unsigned yy=s->y+(s->orientation==WARSHIP_ORIENT_VERTICAL?n:0u);
                f.ships[yy*10u+xx]=(uint8_t)(s->id+1u);
            }
        }
    } else if (p->battle.has_selection && !ui->pending) {
        if (p->battle.target_x >= 10u || p->battle.target_y >= 10u)
            return WARSHIP_COMBAT_BAD_ARGUMENT;
        f.has_selection=1u;
        f.target_x=p->battle.target_x;
        f.target_y=p->battle.target_y;
    }
    const WarshipBattleShot *shots = f.view==WARSHIP_COMBAT_ATTACK ?
        p->battle.history.outgoing : p->battle.history.incoming;
    const unsigned count = f.view==WARSHIP_COMBAT_ATTACK ?
        p->battle.history.outgoing_count : p->battle.history.incoming_count;
    for (unsigned i=0u; i<count; ++i) {
        if (!shot_ok(shots[i])) return WARSHIP_COMBAT_BAD_ARGUMENT;
        const unsigned idx=shots[i].y*10u+shots[i].x;
        if (f.attacks[idx]) return WARSHIP_COMBAT_BAD_ARGUMENT;
        f.attacks[idx]=(uint8_t)(shots[i].impact+1u);
    }
    if (p->battle.history.outgoing_count) {
        f.last_human=p->battle.history.outgoing[p->battle.history.outgoing_count-1u];
        f.has_human=1u;
    }
    if (f.view==WARSHIP_COMBAT_DEFENSE &&
        p->battle.history.incoming_count) {
        f.last_cpu=p->battle.history.incoming[p->battle.history.incoming_count-1u];
        f.has_cpu=1u;
    } else if (f.view==WARSHIP_COMBAT_ATTACK && ui->has_summary) {
        f.last_cpu=ui->last_cpu;
        f.has_cpu=1u;
        f.alert=ui->alert;
    }
    if (include_pending) {
        const WarshipBattlePair *pair=&ui->pending_pair;
        if (!shot_ok(pair->human) || !shot_ok(pair->captain) ||
            f.attacks[pair->human.y*10u+pair->human.x] ||
            pair->round != (uint8_t)(p->battle.history.outgoing_count+1u) ||
            pair->verdict > WARSHIP_BATTLE_DRAW)
            return WARSHIP_COMBAT_BAD_ARGUMENT;
        f.attacks[pair->human.y*10u+pair->human.x]=(uint8_t)(pair->human.impact+1u);
        f.last_human=pair->human;
        f.last_cpu=pair->captain;
        f.has_human=f.has_cpu=1u;
        f.alert=warship_combat_max_alert(ui->alert,pair->captain.impact);
        f.verdict=pair->verdict;
        f.rounds=pair->round;
        f.terminal=(uint8_t)(pair->verdict != WARSHIP_BATTLE_CONTINUE);
    }
    *out=f;
    return WARSHIP_COMBAT_OK;
}

WarshipCombatStatus warship_combat_presented(
    WarshipCombatUi *ui, WarshipLifecycle *game,
    WarshipLifecycleEvent *event)
{
    if (ui==NULL || game==NULL || event==NULL || !ui->pending)
        return WARSHIP_COMBAT_BAD_ARGUMENT;
    WarshipLifecycleEvent emitted=WARSHIP_LIFECYCLE_EVENT_NONE;
    if (warship_lifecycle_ack_reveal(game,&emitted) != WARSHIP_LIFECYCLE_OK)
        return WARSHIP_COMBAT_GAME_ERROR;
    ui->last_cpu=ui->pending_pair.captain;
    ui->has_summary=1u;
    ui->alert=warship_combat_max_alert(ui->alert,ui->last_cpu.impact);
    ui->pending=0u;
    ++ui->reveals;
    ui->terminal_event=(uint8_t)emitted;
    *event=emitted;
    return WARSHIP_COMBAT_OK;
}
