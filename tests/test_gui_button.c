#include "gui_button.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
static GuiButton b[2]; static GuiButtons g;
static GuiButtonEvent ev(GuiInput t,int x,int y){return gui_buttons_handle(&g,t,x,y);}
static void setup(void){
 assert(gui_button_init(&b[0],(GuiRect){14,198,140,32},(GuiRect){8,190,152,46},1,true));
 assert(gui_button_init(&b[1],(GuiRect){166,199,140,30},(GuiRect){160,190,152,46},2,true));
 assert(gui_buttons_init(&g,b,2));
}
static void clicks(void){
 setup();assert(ev(GUI_INPUT_DOWN,9,190).redraw);
 assert(b[0].state==GUI_BUTTON_PRESSED);
 assert(!ev(GUI_INPUT_DOWN,170,205).activated);
 assert(!ev(GUI_INPUT_MOVE,120,214).activated);
 GuiButtonEvent a=ev(GUI_INPUT_UP,120,214);
 assert(a.redraw&&a.activated&&a.action_id==1);
 assert(!ev(GUI_INPUT_UP,120,214).activated);
 assert(!ev(GUI_INPUT_MOVE,120,214).activated);
 assert(b[0].state==GUI_BUTTON_NORMAL);
 (void)ev(GUI_INPUT_DOWN,160,200);
 a=ev(GUI_INPUT_UP,160,200);assert(a.activated&&a.action_id==2);
 (void)ev(GUI_INPUT_DOWN,159,200);
 a=ev(GUI_INPUT_UP,159,200);assert(a.activated&&a.action_id==1);
}
static void cancels(void){
 setup();(void)ev(GUI_INPUT_DOWN,120,200);
 GuiButtonEvent a=ev(GUI_INPUT_MOVE,160,200);
 assert(a.cancelled&&a.redraw&&b[0].state==GUI_BUTTON_NORMAL);
 assert(!ev(GUI_INPUT_MOVE,180,200).activated);
 assert(!ev(GUI_INPUT_UP,180,200).activated);
 (void)ev(GUI_INPUT_DOWN,200,210);
 a=ev(GUI_INPUT_INVALID,0,0);assert(a.redraw&&a.cancelled);
 (void)ev(GUI_INPUT_DOWN,120,210);
 assert(!ev(GUI_INPUT_UP,200,210).activated);
 (void)ev(GUI_INPUT_DOWN,20,220);
 assert(ev(GUI_INPUT_CANCEL,0,0).cancelled);
 assert(!ev(GUI_INPUT_UP,20,220).activated);
 (void)ev(GUI_INPUT_DOWN,4,200);
 (void)ev(GUI_INPUT_MOVE,20,220);
 assert(!ev(GUI_INPUT_UP,20,220).activated);
}
static void disabled(void){
 setup();(void)ev(GUI_INPUT_DOWN,20,210);
 GuiButtonEvent a=gui_buttons_set_enabled(&g,0,false);
 assert(a.redraw&&a.cancelled&&b[0].state==GUI_BUTTON_DISABLED);
 assert(!ev(GUI_INPUT_UP,20,210).activated);
 (void)ev(GUI_INPUT_DOWN,20,210);
 assert(!ev(GUI_INPUT_UP,20,210).activated);
 (void)gui_buttons_set_enabled(&g,0,true);
 (void)ev(GUI_INPUT_DOWN,20,210);
 assert(ev(GUI_INPUT_UP,20,210).activated);
 (void)ev(GUI_INPUT_DOWN,200,210);
 assert(ev(GUI_INPUT_UP,200,210).action_id==2);
 assert(!gui_buttons_set_enabled(&g,4,false).redraw);
}
static void geometry(void){
 setup();assert(gui_rect_contains(b[0].hit,159,235));
 assert(!gui_rect_contains(b[0].hit,160,235));
 assert(!gui_rect_contains(b[0].hit,20,236));
 GuiRect out={0};assert(gui_rect_clip((GuiRect){-5,237,20,20},320,240,&out));
 assert(out.x==0&&out.y==237&&out.w==15&&out.h==3);
 assert(!gui_rect_clip((GuiRect){500,400,5,5},320,240,&out));
 assert(!gui_rect_clip((GuiRect){INT32_MIN,0,INT32_MAX,8},320,240,&out));
 assert(gui_rect_clip((GuiRect){319,239,INT32_MAX,INT32_MAX},320,240,&out));
 assert(out.w==1&&out.h==1);
 assert(!gui_buttons_init(&g,b,0));
 b[1].hit.x=159;assert(!gui_buttons_init(&g,b,2));
}
int main(void){clicks();cancels();disabled();geometry();puts("GUI01_GESTURES_PASS");return 0;}
