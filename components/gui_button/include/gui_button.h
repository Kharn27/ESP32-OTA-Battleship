/* GUI-01: C99 gesture state, independent of renderer and touch driver. */
#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
typedef struct { int32_t x,y,w,h; } GuiRect;
typedef enum { GUI_BUTTON_NORMAL=0, GUI_BUTTON_PRESSED, GUI_BUTTON_DISABLED } GuiButtonState;
typedef enum { GUI_INPUT_DOWN=0, GUI_INPUT_MOVE, GUI_INPUT_UP, GUI_INPUT_CANCEL, GUI_INPUT_INVALID } GuiInput;
typedef struct { GuiRect visual,hit; uint16_t action_id; GuiButtonState state; } GuiButton;
typedef struct { GuiButton *items; size_t count,owner; bool tracking; } GuiButtons;
typedef struct { bool redraw,activated,cancelled; size_t index; uint16_t action_id; } GuiButtonEvent;
bool gui_rect_contains(GuiRect r,int32_t x,int32_t y);
bool gui_rect_clip(GuiRect r,int32_t width,int32_t height,GuiRect *out);
bool gui_button_init(GuiButton *b,GuiRect visual,GuiRect hit,uint16_t action_id,bool enabled);
bool gui_buttons_init(GuiButtons *group,GuiButton *buttons,size_t count);
GuiButtonEvent gui_buttons_handle(GuiButtons *group,GuiInput input,int32_t x,int32_t y);
GuiButtonEvent gui_buttons_set_enabled(GuiButtons *group,size_t index,bool enabled);
#endif
