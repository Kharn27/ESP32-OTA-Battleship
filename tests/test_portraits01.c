/* #128 C99: actual frozen candidate data, alpha, clipping, DMA sentinel.
 * Dump exact 320x240 RGB565BE composed by this same C module for an
 * independent PC PNG comparison (Python reads source preview as oracle).
 */
#include "portraits01.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint16_t rgb565(unsigned r,unsigned g,unsigned b)
{
 return (uint16_t)(((r>>3)<<11)|((g>>2)<<5)|(b>>3));
}
static uint16_t independent_blend(uint16_t bg,uint16_t ink,unsigned a)
{
 if(a==0u)return bg;
 if(a==255u)return ink;
 const unsigned inv=255u-a;
 const unsigned r=((((unsigned)(bg>>11)&31u)*inv+
                      ((unsigned)(ink>>11)&31u)*a+127u)/255u);
 const unsigned g=((((unsigned)(bg>>5)&63u)*inv+
                      ((unsigned)(ink>>5)&63u)*a+127u)/255u);
 const unsigned b=((unsigned)(bg&31u)*inv+(unsigned)(ink&31u)*a+127u)/255u;
 return (uint16_t)((r<<11)|(g<<5)|b);
}
static uint8_t frame[P01_W*P01_H*2u];
static uint16_t at(unsigned x,unsigned y)
{
 const size_t p=((size_t)y*P01_W+x)*2u;
 return (uint16_t)(((uint16_t)frame[p]<<8)|frame[p+1u]);
}
int main(int argc,char **argv)
{
 assert(P01_BAND_BYTES==5120u);
 uint8_t band[P01_BAND_BYTES+17u];
 memset(band,0xa5,sizeof band);
 assert(portrait01_compose_band(0u,8u,NULL,P01_BAND_BYTES)==-1);
 assert(portrait01_compose_band(0u,0u,band,sizeof band)==-1);
 assert(portrait01_compose_band(0u,9u,band,sizeof band)==-1);
 assert(portrait01_compose_band(240u,1u,band,sizeof band)==-1);
 assert(portrait01_compose_band(239u,2u,band,sizeof band)==-1);
 assert(portrait01_compose_band(0u,8u,band,P01_BAND_BYTES-1u)==-1);
 for(size_t i=0u;i<sizeof band;++i)assert(band[i]==0xa5u);
 const unsigned sizes[P01_COUNT]={32u,48u,64u};
 unsigned opaque=0u,transparent=0u,partial=0u;
 for(unsigned i=0;i<P01_COUNT;++i){
  const Portrait01Sprite *s=&portrait01_sprites[i];
  assert(s->pixels && s->source_sha256);
  assert(strlen(s->source_sha256)==64u);
  assert(s->width==sizes[i] && s->height==sizes[i]);
  assert(s->bytes==(size_t)sizes[i]*sizes[i]*3u);
  for(size_t p=0;p<s->bytes;p+=3u){
   const uint8_t a=s->pixels[p+2u];
   const uint16_t ink=(uint16_t)(((uint16_t)s->pixels[p]<<8)|s->pixels[p+1u]);
   const uint16_t bg=rgb565(246u,237u,220u);
   const uint16_t actual=portrait01_blend565(bg,ink,a);
   assert(actual==independent_blend(bg,ink,a));
   if(a==0u){++transparent;assert(actual==bg);}
   else if(a==255u){++opaque;assert(actual==ink);}
   else ++partial;
  }
 }
 /* The candidate 32px source has NO alpha=255 pixels; that is source
  * evidence, not a reason to edit the artwork. Test alpha255 synthetically. */
 assert(transparent && partial);
 assert(portrait01_blend565(0x0000u,0xffffu,255u)==0xffffu);
 assert(portrait01_blend565(0xffffu,0x0000u,0u)==0xffffu);
 assert(portrait01_blend565(0u,0xffffu,128u)==independent_blend(0u,0xffffu,128u));
 for(unsigned y=0;y<P01_H;y+=P01_BAND_ROWS){
  memset(band,0xa5,sizeof band);
  assert(portrait01_compose_band(y,P01_BAND_ROWS,band,sizeof band)==0);
  for(size_t i=P01_BAND_BYTES;i<sizeof band;++i)assert(band[i]==0xa5u);
  memcpy(frame+(size_t)y*P01_W*2u,band,P01_BAND_BYTES);
 }
 /* Known PNG compositor geometry and solid RGB565 colors outside sprites. */
 assert(at(0u,0u)==rgb565(12u,21u,31u));
 assert(at(5u,20u)==rgb565(29u,42u,59u));
 assert(at(5u,132u)==rgb565(232u,224u,207u));
 assert(at(10u,27u)==rgb565(12u,30u,47u));
 assert(at(10u,139u)==rgb565(246u,237u,220u));
 assert(at(9u,6u)==rgb565(237u,216u,166u)); /* 'A' top heading */
 assert(at(9u,119u)==rgb565(62u,74u,76u)); /* 'A' bottom heading */
 /* Exact 0/partial/255 alpha for ACTUAL imported sprites on BOTH backgrounds. */
 for(unsigned i=0;i<P01_COUNT;++i) {
  const Portrait01Sprite *s=&portrait01_sprites[i];
  const unsigned px=9u+103u*i+(94u-s->width)/2u;
  const unsigned ys[2]={110u-s->height,220u-s->height};
  const uint16_t backgrounds[2]={rgb565(12u,30u,47u),rgb565(246u,237u,220u)};
  for(unsigned side=0u;side<2u;++side)
   for(unsigned y=0u;y<s->height;++y)
    for(unsigned x=0u;x<s->width;++x){
     const size_t pos=((size_t)y*s->width+x)*3u;
     const uint16_t ink=(uint16_t)(((uint16_t)s->pixels[pos]<<8)|s->pixels[pos+1u]);
     const uint8_t alpha=s->pixels[pos+2u];
     assert(at(px+x,ys[side]+y)==
            independent_blend(backgrounds[side],ink,alpha));
    }
 }
 printf("PORTRAITS01_PIXELS_PASS opaque=%u transparent=%u partial=%u "
        "three_sizes=32,48,64 two_backgrounds=dark,light full_frame=320x240\n",
        opaque,transparent,partial);
 if(argc==3 && strcmp(argv[1],"--dump")==0){
  FILE *f=fopen(argv[2],"wb");
  if(!f){perror("dump open");return 1;}
  if(fwrite(frame,1u,sizeof frame,f)!=sizeof frame){fclose(f);return 2;}
  if(fclose(f)!=0)return 3;
 }else if(argc!=1){fprintf(stderr,"usage: test [--dump file]\n");return 4;}
 return 0;
}
