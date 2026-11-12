/* Compare all G01 pixels to approved NORMAL composer and pressed oracle. */
#include "d15a_g01.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint8_t base[D15A_BAND_BYTES+16],rect[D15A_BAND_BYTES+16];
static uint16_t read565(const uint8_t *p){
 return (uint16_t)(((uint16_t)p[0]<<8)|p[1]);
}
int main(void){
 assert(d15a_dim565(0xf800u,0)==0);
 assert(d15a_dim565(0x07e0u,255)==0x07e0u);
 assert(d15a_dim565(0xffffu,0)==0);
 assert(d15a_dim565(0xffffu,255)==0xffffu);
 assert(d15a_blend565(0x1234u,0xffffu,0)==0x1234u);
 assert(d15a_blend565(0x1234u,0xffffu,255)==0xffffu);
 assert(d15a_blend565(0,0xffffu,128)==0x8410u);
 memset(rect,0xa5,sizeof rect);
 assert(d15a_compose_region(319,239,2,1,D15A_IMAGE_COUNT,170,rect,D15A_BAND_BYTES)==-1);
 assert(d15a_compose_region(0,0,320,9,D15A_IMAGE_COUNT,170,rect,D15A_BAND_BYTES)==-1);
 assert(d15a_compose_region(0,0,320,8,D15A_IMAGE_COUNT,170,rect,1)==-1);
 for(size_t i=0;i<sizeof rect;++i)assert(rect[i]==0xa5);
 unsigned changed=0,transparent=0,partial=0;
 for(unsigned y=0;y<D15A_H;y+=8){
  assert(d15a_compose_band((uint16_t)y,8,base,D15A_BAND_BYTES)==0);
  assert(d15a_compose_region(0,(uint16_t)y,320,8,D15A_IMAGE_COUNT,170,rect,D15A_BAND_BYTES)==0);
  assert(memcmp(base,rect,D15A_BAND_BYTES)==0);
  for(unsigned layer=D15A_JOUER;layer<=D15A_PALMARES;++layer){
   const D15AAsset *s=&d15a_assets[layer];
   unsigned y0=y>s->y?y:s->y;
   unsigned y1=y+8<(unsigned)s->y+s->h?y+8:(unsigned)s->y+s->h;
   if(y0>=y1)continue;
   const unsigned h=y1-y0;
   memset(rect,0xa5,sizeof rect);
   assert(d15a_compose_region(s->x,(uint16_t)y0,s->w,(uint16_t)h,layer,170,rect,D15A_BAND_BYTES)==0);
   for(unsigned row=0;row<h;++row)for(unsigned col=0;col<s->w;++col){
    const unsigned xx=s->x+col,yy=y0+row;
    const size_t si=((size_t)(yy-s->y)*s->w+col)*3u;
    const uint8_t a=s->pixels[si+2u];
    const uint16_t normal=read565(base+((size_t)(yy-y)*D15A_W+xx)*2u);
    const uint16_t pressed=read565(rect+((size_t)row*s->w+col)*2u);
    const uint16_t wallpaper=read565(d15a_assets[0].pixels+((size_t)yy*D15A_W+xx)*2u);
    const uint16_t ink=read565(s->pixels+si);
    assert(pressed==d15a_blend565(wallpaper,d15a_dim565(ink,170),a));
    if(!a){assert(normal==pressed&&pressed==wallpaper);++transparent;}
    if(a>0u&&a<255u)++partial;
    if(normal!=pressed)++changed;
   }
   for(size_t i=(size_t)s->w*h*2u;i<sizeof rect;++i)assert(rect[i]==0xa5);
  }
 }
 assert(changed>0&&transparent>0&&partial>0);
 printf("GUI01_PIXELS_PASS normal=76800_exact changed=%u alpha0=%u alpha_partial=%u\n",changed,transparent,partial);
 return 0;
}
