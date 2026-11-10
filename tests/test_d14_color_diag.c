/* D14 HOST: exact, byte-addressable RGB565 BE and five isolated profiles.
 * No ESP32 required. Any change in emitted reference values must fail CI.
 */
#include "d14_color_diag.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(q) do {if(!(q)){fprintf(stderr,"D14 COLOR FAIL line %d: %s\n",__LINE__,#q);return 1;}}while(0)

static uint8_t band[D14_COLOR_BAND_BYTES];
static int pixel_is(unsigned x,unsigned row,uint16_t rgb)
{
    size_t at=((size_t)row*D14_COLOR_W+x)*2u;
    return band[at]==(uint8_t)(rgb>>8) &&
           band[at+1u]==(uint8_t)rgb;
}

int main(void)
{
    const D14ColorProfile expected[5] = {
        {'A',0,0,0},{'B',1,0,0},{'C',0,1,0},
        {'D',1,0,1},{'E',1,1,1}
    };
    D14ColorProfile p={'Z',7,7,7};
    for(unsigned i=0;i<D14_COLOR_PROFILES;++i) {
        CHECK(d14_color_profile(i,&p)==0);
        CHECK(memcmp(&p,&expected[i],sizeof p)==0);
    }
    CHECK(d14_color_profile(5u,&p)!=0);
    CHECK(p.id=='E');
    CHECK(d14_color_profile(0u,NULL)!=0);

    static const uint8_t reference[15]={
        0x00,0x15,0x17,0x07,0x11,0x06,0x2b,0x56,
        0x3c,0x05,0x10,0x0f,0x3f,0x3f,0x0f
    };
    CHECK(sizeof d14_color_doom_gamma==15u);
    CHECK(memcmp(d14_color_doom_gamma,reference,15u)==0);

    memset(band,0xa5,sizeof band);
    CHECK(d14_color_direct_band(0u,8u,band,sizeof band)==0);
    CHECK(pixel_is(0u,0u,0x0000u));   /* black=00:00 */
    CHECK(pixel_is(79u,7u,0x0000u));
    CHECK(pixel_is(80u,0u,0xffffu)); /* white=ff:ff */
    CHECK(pixel_is(159u,7u,0xffffu));
    CHECK(pixel_is(160u,0u,0xf800u)); /* red=f8:00 */
    CHECK(pixel_is(239u,7u,0xf800u));
    CHECK(pixel_is(240u,0u,0x07e0u)); /* green=07:e0 */
    CHECK(pixel_is(319u,7u,0x07e0u));

    CHECK(d14_color_direct_band(120u,8u,band,sizeof band)==0);
    CHECK(pixel_is(0u,0u,0x001fu)); /* blue=00:1f */
    CHECK(pixel_is(80u,0u,0x8410u)); /* gray=84:10 */
    CHECK(pixel_is(160u,0u,0x0000u)); /* ramp dark */
    CHECK(pixel_is(239u,0u,0xffffu)); /* ramp bright */
    CHECK(pixel_is(240u,0u,0xffe0u)); /* yellow=ff:e0 */
    CHECK(pixel_is(319u,7u,0xffe0u));
    CHECK(d14_color_direct_pixel(320u,0u)==0u);
    CHECK(d14_color_direct_pixel(0u,240u)==0u);
    CHECK(d14_color_direct_pixel(0u,120u)==0x001fu);

    /* Full last band must be addressable without overrun. */
    CHECK(d14_color_direct_band(232u,8u,band,sizeof band)==0);
    CHECK(pixel_is(319u,7u,0xffe0u));

    memset(band,0x6c,sizeof band);
    CHECK(d14_color_direct_band(232u,8u,band,sizeof band-1u)!=0);
    CHECK(band[0]==0x6cu && band[sizeof band-1u]==0x6cu);
    CHECK(d14_color_direct_band(233u,8u,band,sizeof band)!=0);
    CHECK(d14_color_direct_band(240u,1u,band,sizeof band)!=0);
    CHECK(d14_color_direct_band(0u,9u,band,sizeof band)!=0);
    CHECK(d14_color_direct_band(0u,0u,band,sizeof band)!=0);
    CHECK(d14_color_direct_band(0u,8u,NULL,sizeof band)!=0);
    CHECK(d14_color_direct_band(0u,8u,band,0u)!=0);
    puts("D14 COLOR PASS: 5 independent profiles, exact gamma,"
         " 8 direct RGB565BE swatches, ramp, bounded DMA");
    return 0;
}
