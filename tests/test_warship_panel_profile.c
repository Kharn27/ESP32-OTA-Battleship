#include "warship_panel_profile.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Independent command/order oracle for TFT_eSPI V2.5.43 ILI9341_2.
 * Deliberately NOT generated from the target init array.
 */
static void test_init_sequence(void)
{
    static const uint8_t ids[] = {
        0xCF,0xED,0xE8,0xCB,0xF7,0xEA,0xC0,0xC1,0xC5,0xC7,
        0x3A,0x36,0xB1,0xB6,0xF2,0x26,0xE0,0xE1,0x2B,0x2A,
        0x11,0x29,0x21
    };
    size_t n=0;
    assert(warship_panel_init_commands(NULL)==NULL);
    const WarshipPanelCmd *v=warship_panel_init_commands(&n);
    assert(v != NULL && n == sizeof ids);
    for(size_t i=0u;i<n;++i) {
        assert(v[i].cmd==ids[i] && v[i].len<=15u);
        assert(v[i].delay_ms==(i==20u?120u:0u));
    }
    assert(v[0].len==3u && memcmp(v[0].data,(const uint8_t[]){0,0xC1,0x30},3)==0);
    assert(v[1].len==4u && memcmp(v[1].data,(const uint8_t[]){0x64,3,0x12,0x81},4)==0);
    assert(v[6].len==1u && v[6].data[0]==0x10u);
    assert(v[7].len==1u && v[7].data[0]==0x00u);
    assert(v[8].len==2u && v[8].data[0]==0x30u && v[8].data[1]==0x30u);
    assert(v[9].data[0]==0xB7u);
    assert(v[10].data[0]==0x55u);
    assert(v[11].data[0]==0x08u);
    assert(v[12].len==2u && v[12].data[0]==0u && v[12].data[1]==0x1Au);
    assert(v[13].len==3u && memcmp(v[13].data,(const uint8_t[]){8,0x82,0x27},3)==0);
    assert(v[16].len==15u &&
           memcmp(v[16].data,(const uint8_t[]){0x0F,0x2A,0x28,8,0x0E,8,0x54,0xA9,0x43,0x0A,0x0F,0,0,0,0},15)==0);
    assert(v[17].len==15u &&
           memcmp(v[17].data,(const uint8_t[]){0,0x15,0x17,7,0x11,6,0x2B,0x56,0x3C,5,0x10,0x0F,0x3F,0x3F,0x0F},15)==0);
    assert(v[18].len==4u && memcmp(v[18].data,(const uint8_t[]){0,0,1,0x3F},4)==0);
    assert(v[19].len==4u && memcmp(v[19].data,(const uint8_t[]){0,0,0,0xEF},4)==0);
    assert(v[20].len==0u && v[21].len==0u && v[22].len==0u);
}
static void test_runtime(void)
{
    const uint8_t ids[]={0xF2,0x26,0xE0,0xE1,0x21,0x21};
    const uint8_t gamma[]={0,0x15,0x17,7,0x11,6,0x2B,0x56,0x3C,5,0x10,0x0F,0x3F,0x3F,0x0F};
    size_t n=0;
    assert(warship_panel_runtime_commands(NULL)==NULL);
    const WarshipPanelCmd *v=warship_panel_runtime_commands(&n);
    assert(v!=NULL && n==sizeof ids);
    for(size_t i=0u;i<n;++i) {
        assert(v[i].cmd==ids[i] && v[i].delay_ms==0u);
    }
    assert(v[0].len==1u && v[0].data[0]==0u);
    assert(v[1].len==1u && v[1].data[0]==1u);
    assert(v[2].len==15u && memcmp(v[2].data,gamma,15)==0);
    assert(v[3].len==15u && memcmp(v[3].data,gamma,15)==0);
    assert(v[4].len==0u && v[5].len==0u);
    assert(warship_panel_landscape_madctl()==0x28u);
}
static void test_native_landscape_windows(void)
{
    WarshipPanelWindow w;
    assert(warship_panel_window(0,0,320,8,&w)==0);
    assert(memcmp(w.caset,(const uint8_t[]){0,0,1,0x3F},4)==0);
    assert(memcmp(w.raset,(const uint8_t[]){0,0,0,7},4)==0);
    assert(warship_panel_window(0,232,320,8,&w)==0);
    assert(memcmp(w.caset,(const uint8_t[]){0,0,1,0x3F},4)==0);
    assert(memcmp(w.raset,(const uint8_t[]){0,232,0,239},4)==0);
    assert(warship_panel_window(319,239,1,1,&w)==0);
    assert(memcmp(w.caset,(const uint8_t[]){1,0x3F,1,0x3F},4)==0);
    assert(memcmp(w.raset,(const uint8_t[]){0,239,0,239},4)==0);
    assert(warship_panel_window(79,119,2,2,&w)==0);
    assert(memcmp(w.caset,(const uint8_t[]){0,79,0,80},4)==0);
    assert(memcmp(w.raset,(const uint8_t[]){0,119,0,120},4)==0);
    assert(warship_panel_window(0,0,1,1,NULL)==-1);
    assert(warship_panel_window(0,0,0,8,&w)==-1);
    assert(warship_panel_window(0,0,320,0,&w)==-1);
    assert(warship_panel_window(1,0,320,8,&w)==-1);
    assert(warship_panel_window(0,1,320,240,&w)==-1);
    assert(warship_panel_window(320,0,1,1,&w)==-1);
    assert(warship_panel_window(0,240,1,1,&w)==-1);
    assert(warship_panel_window(UINT16_MAX,0,1,1,&w)==-1);
    assert(warship_panel_window(0,UINT16_MAX,1,1,&w)==-1);
}

int main(void)
{
    test_init_sequence();
    test_runtime();
    test_native_landscape_windows();
    puts("WARSHIP_PANEL_PROFILE production init/runtime/MADCTL/windows: PASS");
    return 0;
}
