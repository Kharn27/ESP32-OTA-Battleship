#include "gui_button.h"
#include <stdint.h>
bool gui_rect_contains(GuiRect r, int32_t x, int32_t y)
{
    return r.w > 0 && r.h > 0 && (int64_t)x >= r.x && (int64_t)y >= r.y &&
           (int64_t)x < (int64_t)r.x + r.w &&
           (int64_t)y < (int64_t)r.y + r.h;
}
bool gui_rect_clip(GuiRect r, int32_t width, int32_t height, GuiRect *out)
{
    if (!out || width <= 0 || height <= 0 || r.w <= 0 || r.h <= 0)
        return false;
    const int64_t x0 = r.x > 0 ? r.x : 0, y0 = r.y > 0 ? r.y : 0;
    const int64_t xx = (int64_t)r.x + r.w, yy = (int64_t)r.y + r.h;
    const int64_t x1 = xx < width ? xx : width, y1 = yy < height ? yy : height;
    if (x1 <= x0 || y1 <= y0) return false;
    *out = (GuiRect){(int32_t)x0,(int32_t)y0,(int32_t)(x1-x0),(int32_t)(y1-y0)};
    return true;
}
bool gui_button_init(GuiButton *b, GuiRect visual, GuiRect hit,
                     uint16_t action_id, bool enabled)
{
    if (!b || visual.w <= 0 || visual.h <= 0 || hit.w <= 0 || hit.h <= 0 ||
        action_id == 0) return false;
    *b = (GuiButton){visual, hit, action_id,
                      enabled ? GUI_BUTTON_NORMAL : GUI_BUTTON_DISABLED};
    return true;
}
bool gui_buttons_init(GuiButtons *group, GuiButton *buttons, size_t count)
{
    if (!group || !buttons || !count) return false;
    for (size_t i=0; i<count; ++i) {
        if (buttons[i].hit.w <= 0 || buttons[i].hit.h <= 0 ||
            buttons[i].visual.w <= 0 || buttons[i].visual.h <= 0 ||
            buttons[i].action_id == 0) return false;
        for (size_t j=0; j<i; ++j) {
            const GuiRect a=buttons[i].hit, b=buttons[j].hit;
            if ((int64_t)a.x < (int64_t)b.x+b.w &&
                (int64_t)b.x < (int64_t)a.x+a.w &&
                (int64_t)a.y < (int64_t)b.y+b.h &&
                (int64_t)b.y < (int64_t)a.y+a.h) return false;
            if (buttons[i].action_id == buttons[j].action_id) return false;
        }
    }
    *group = (GuiButtons){buttons,count,count,false};
    return true;
}
static GuiButtonEvent reset_owner(GuiButtons *group, bool cancelled)
{
    GuiButtonEvent e={0};
    if (group->owner < group->count) {
        e.index=group->owner;
        if (group->items[e.index].state == GUI_BUTTON_PRESSED) {
            group->items[e.index].state=GUI_BUTTON_NORMAL;
            e.redraw=true;
        }
        group->owner=group->count;
        e.cancelled=cancelled;
    }
    return e;
}
GuiButtonEvent gui_buttons_handle(GuiButtons *group, GuiInput input,
                                  int32_t x, int32_t y)
{
    GuiButtonEvent e={0};
    if (!group || !group->items || !group->count || group->owner>group->count)
        return e;
    if (input == GUI_INPUT_DOWN) {
        if (group->tracking) return e;
        group->tracking=true;
        for (size_t i=0;i<group->count;++i) {
            if (group->items[i].state==GUI_BUTTON_NORMAL &&
                gui_rect_contains(group->items[i].hit,x,y)) {
                group->owner=i;
                group->items[i].state=GUI_BUTTON_PRESSED;
                e.redraw=true; e.index=i;
                break;
            }
        }
        return e;
    }
    if (input==GUI_INPUT_INVALID || input==GUI_INPUT_CANCEL) {
        e=reset_owner(group,true);
        return e; /* blocked until UP, even if next MOVE enters a neighbour */
    }
    if (!group->tracking) return e;
    if (input==GUI_INPUT_MOVE) {
        if (group->owner<group->count &&
            !gui_rect_contains(group->items[group->owner].hit,x,y))
            return reset_owner(group,true);
        return e;
    }
    if (input==GUI_INPUT_UP) {
        if (group->owner < group->count) {
            const size_t owner=group->owner;
            const bool valid=gui_rect_contains(group->items[owner].hit,x,y) &&
                             group->items[owner].state==GUI_BUTTON_PRESSED;
            e=reset_owner(group,!valid);
            if (valid) {e.activated=true;e.action_id=group->items[owner].action_id;}
        }
        group->tracking=false;
    }
    return e;
}
GuiButtonEvent gui_buttons_set_enabled(GuiButtons *group, size_t index,
                                       bool enabled)
{
    GuiButtonEvent e={0};
    if (!group || !group->items || index >= group->count) return e;
    if (enabled && group->owner==index) return e;
    if (!enabled && group->owner==index) e=reset_owner(group,true);
    GuiButton *b=&group->items[index];
    const GuiButtonState next=enabled?GUI_BUTTON_NORMAL:GUI_BUTTON_DISABLED;
    if (b->state!=next) {b->state=next;e.redraw=true;e.index=index;}
    return e;
}
