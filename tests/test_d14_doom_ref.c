#include "d14_doom_ref.h"
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
    assert(d14_doom_init(NULL)==NULL);
    const D14DoomCmd *v=d14_doom_init(&n);
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
    assert(d14_doom_runtime(NULL)==NULL);
    const D14DoomCmd *v=d14_doom_runtime(&n);
    assert(v!=NULL && n==sizeof ids);
    for(size_t i=0u;i<n;++i) {
        assert(v[i].cmd==ids[i] && v[i].delay_ms==0u);
    }
    assert(v[0].len==1u && v[0].data[0]==0u);
    assert(v[1].len==1u && v[1].data[0]==1u);
    assert(v[2].len==15u && memcmp(v[2].data,gamma,15)==0);
    assert(v[3].len==15u && memcmp(v[3].data,gamma,15)==0);
    assert(v[4].len==0u && v[5].len==0u);
    assert(d14_doom_case10_madctl()==0x88u);
}
static void test_rgb565be(void)
{
    uint8_t data[D14_DOOM_BAND_BYTES+2u];
    memset(data,0xA5,sizeof data);
    assert(d14_doom_midtone_band(0,8,data,sizeof data)==0);
    assert(data[D14_DOOM_BAND_BYTES]==0xA5 && data[D14_DOOM_BAND_BYTES+1]==0xA5);
    for(unsigned y=0u;y<8u;++y)
        for(unsigned x=0u;x<320u;++x) {
            const uint16_t expected=d14_doom_midtone_pixel((uint16_t)x,(uint16_t)y);
            const size_t off=(y*320u+x)*2u;
            assert(data[off]==(uint8_t)(expected>>8));
            assert(data[off+1u]==(uint8_t)expected);
        }
    assert(d14_doom_midtone_pixel(0,120)==0xB1C4u); /* R176 G56 B38 */
    assert(d14_doom_midtone_pixel(80,120)==0x1A0Bu); /* R24 G66 B93 */
    assert(d14_doom_midtone_pixel(160,120)==0x4942u);
    assert(d14_doom_midtone_pixel(240,120)==0xDC44u);
    assert(d14_doom_midtone_pixel(320,0)==0u);
    assert(d14_doom_midtone_pixel(0,240)==0u);
    assert(d14_doom_midtone_band(0,0,data,sizeof data)==-1);
    assert(d14_doom_midtone_band(0,9,data,sizeof data)==-1);
    assert(d14_doom_midtone_band(239,8,data,sizeof data)==-1);
    assert(d14_doom_midtone_band(240,1,data,sizeof data)==-1);
    assert(d14_doom_midtone_band(0,8,NULL,sizeof data)==-1);
    assert(d14_doom_midtone_band(0,8,data,D14_DOOM_BAND_BYTES-1u)==-1);
    assert(d14_doom_midtone_band(239,1,data,640u)==0);
}
int main(void)
{
    test_init_sequence();
    test_runtime();
    test_rgb565be();
    puts("D14_DOOM_REF sequence + RGB565BE: PASS");
    return 0;
}
