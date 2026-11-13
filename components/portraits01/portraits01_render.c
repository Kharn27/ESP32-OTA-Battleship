/* #128 TEST ONLY: same coordinates/text/glyphs as PR #127 export.py.
 * Only three independently hashed PNG portraits supply sprite pixels.
 * Recompose directly into one caller-provided RGB565BE DMA band.
 */
#include "portraits01.h"
#include <stdbool.h>
#include <string.h>
typedef struct {char c;uint8_t r[5];} Glyph;
static const Glyph glyphs[]={
 {' ',{0,0,0,0,0}},{'/',{1,1,2,4,4}},
 {'0',{7,5,5,5,7}},{'1',{2,6,2,2,7}},{'2',{6,1,2,4,7}},
 {'3',{6,1,2,1,6}},{'4',{5,5,7,1,1}},{'6',{3,4,7,5,7}},
 {'8',{7,5,7,5,7}},{'A',{2,5,7,5,5}},{'B',{6,5,6,5,6}},
 {'C',{3,4,4,4,3}},{'D',{6,5,5,5,6}},{'E',{7,4,6,4,7}},
 {'F',{7,4,6,4,4}},{'I',{7,2,2,2,7}},{'L',{4,4,4,4,7}},
 {'M',{5,7,7,5,5}},{'N',{5,7,7,7,5}},{'O',{7,5,5,5,7}},
 {'P',{6,5,6,4,4}},{'R',{6,5,6,5,5}},{'S',{3,4,2,1,6}},
 {'T',{7,2,2,2,2}},{'U',{5,5,5,5,7}},{'V',{5,5,5,5,2}},
 {'X',{5,5,2,5,5}}
};
static uint16_t rgb565(unsigned r,unsigned g,unsigned b)
{
 return (uint16_t)(((r>>3)<<11)|((g>>2)<<5)|(b>>3));
}
uint16_t portrait01_blend565(uint16_t bg,uint16_t ink,uint8_t a)
{
 if(a==0u)return bg;
 if(a==255u)return ink;
 const unsigned b=255u-a;
 const unsigned r=((((unsigned)(bg>>11)&31u)*b+
                      ((unsigned)(ink>>11)&31u)*a+127u)/255u);
 const unsigned g=((((unsigned)(bg>>5)&63u)*b+
                      ((unsigned)(ink>>5)&63u)*a+127u)/255u);
 const unsigned bl=(((unsigned)(bg&31u)*b+(unsigned)(ink&31u)*a+127u)/255u);
 return (uint16_t)((r<<11)|(g<<5)|bl);
}
static bool box(unsigned x,unsigned y,unsigned rx,unsigned ry,
                unsigned w,unsigned h)
{
 return x>=rx && y>=ry && x-rx<w && y-ry<h;
}
static bool label(const char *s,unsigned rx,unsigned ry,unsigned x,unsigned y)
{
 if(x<rx || y<ry || y-ry>=5u)return false;
 const unsigned dx=x-rx,col=dx%4u;
 const size_t idx=dx/4u;
 if(col>=3u || idx>=strlen(s))return false;
 for(size_t i=0;i<sizeof glyphs/sizeof glyphs[0];++i)
  if(glyphs[i].c==s[idx])
   return (glyphs[i].r[y-ry] & (uint8_t)(1u<<(2u-col)))!=0u;
 return false;
}
static uint16_t sprite_pixel(const Portrait01Sprite *s,unsigned sx,unsigned sy,
                              unsigned x,unsigned y,uint16_t bg)
{
 if(s==NULL || s->pixels==NULL ||
    !box(x,y,sx,sy,s->width,s->height))return bg;
 const size_t idx=(((size_t)(y-sy)*s->width)+(x-sx))*3u;
 if(idx>s->bytes || s->bytes-idx<3u)return bg;
 const uint16_t fg=(uint16_t)(((uint16_t)s->pixels[idx]<<8)|s->pixels[idx+1u]);
 return portrait01_blend565(bg,fg,s->pixels[idx+2u]);
}
static uint16_t pixel(unsigned x,unsigned y)
{
 uint16_t color=rgb565(12u,21u,31u);
 if(box(x,y,4u,16u,312u,100u))color=rgb565(29u,42u,59u);
 if(box(x,y,4u,128u,312u,100u))color=rgb565(232u,224u,207u);
 for(unsigned i=0;i<P01_COUNT;++i){
  const unsigned lx=9u+103u*i;
  if(box(x,y,lx,26u,94u,87u))color=rgb565(12u,30u,47u);
  if(box(x,y,lx,138u,94u,85u))color=rgb565(246u,237u,220u);
 }
 if(label("A1 / FOND SOMBRE",8u,6u,x,y))color=rgb565(237u,216u,166u);
 if(label("A1 / FOND CLAIR",8u,119u,x,y))color=rgb565(62u,74u,76u);
 static const char *const caps[P01_COUNT]={"32 PX","48 PX","64 PX"};
 for(unsigned i=0;i<P01_COUNT;++i){
  const unsigned lx=9u+103u*i,sz=portrait01_sprites[i].width;
  if(label(caps[i],lx+34u,30u,x,y))color=rgb565(235u,215u,169u);
  if(label(caps[i],lx+34u,142u,x,y))color=rgb565(58u,63u,69u);
  if(sz>94u || portrait01_sprites[i].height!=sz)continue;
  const unsigned sx=lx+(94u-sz)/2u;
  color=sprite_pixel(&portrait01_sprites[i],sx,110u-sz,x,y,color);
  color=sprite_pixel(&portrait01_sprites[i],sx,220u-sz,x,y,color);
 }
 return color;
}
int portrait01_compose_band(unsigned y,unsigned rows,uint8_t *dst,size_t cap)
{
 if(!dst || rows==0u || rows>P01_BAND_ROWS || y>=P01_H ||
    rows>P01_H-y || cap<(size_t)rows*P01_W*2u)return -1;
 for(unsigned dy=0;dy<rows;++dy)for(unsigned x=0;x<P01_W;++x){
  const uint16_t c=pixel(x,y+dy);
  const size_t off=((size_t)dy*P01_W+x)*2u;
  dst[off]=(uint8_t)(c>>8);dst[off+1u]=(uint8_t)c;
 }
 return 0;
}
