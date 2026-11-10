/* D14 COLOR HOST: every one of 76,800 G01 PR90 PNG RGB565 pixels
 * must survive Flash raw encoding and T03 rectangle decoding bit-for-bit.
 * Golden bytes are the pinned PNG quantized directly by the Python
 * build-time exporter (no G03 artwork, CSS or alpha).
 */
#include "t03_decode.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const T03Image d14_color_g01_image;
#define RGB_BYTES (320u*240u*2u)
#define BAND_BYTES (320u*8u*2u)
#define CHECK(q) do { if(!(q)) { \
    fprintf(stderr,"D14 G01 FAIL line %u: %s\\n",(unsigned)__LINE__,#q); \
    return 1; } } while(0)

static uint8_t golden[RGB_BYTES];
static uint8_t band[BAND_BYTES];

int main(int argc,char **argv)
{
    CHECK(argc==2 && argv[1]);
    FILE *fp=fopen(argv[1],"rb");
    CHECK(fp!=NULL);
    CHECK(fread(golden,1,RGB_BYTES,fp)==RGB_BYTES);
    CHECK(fgetc(fp)==EOF); /* no hidden padding/cropped rows */
    CHECK(fclose(fp)==0);
    CHECK(d14_color_g01_image.format==T03_RAW565BE);
    CHECK(d14_color_g01_image.width==320u);
    CHECK(d14_color_g01_image.height==240u);
    CHECK(d14_color_g01_image.row_offsets==NULL);
    CHECK(d14_color_g01_image.row_offset_count==0u);
    CHECK(d14_color_g01_image.bytes!=NULL);

    for (unsigned y=0u;y<240u;y+=8u) {
        memset(band,0xa5,sizeof band);
        CHECK(t03_decode_rect(&d14_color_g01_image,
              0u,(uint16_t)y,320u,8u,band,sizeof band)==0);
        CHECK(memcmp(band,golden+(size_t)y*320u*2u,sizeof band)==0);
    }
    /* Subrectangle crossing non-trivial coordinates and Flash rectangle layout. */
    CHECK(t03_decode_rect(&d14_color_g01_image,
          31u,53u,19u,8u,band,sizeof band)==0);
    for(unsigned y=0u;y<8u;++y)
        CHECK(memcmp(band+y*19u*2u,
              golden+((size_t)(53u+y)*320u+31u)*2u,19u*2u)==0);
    memset(band,0x5a,sizeof band);
    CHECK(t03_decode_rect(&d14_color_g01_image,
          0u,0u,320u,8u,band,BAND_BYTES-1u)!=0);
    CHECK(band[0]==0x5a && band[BAND_BYTES-1u]==0x5a);
    CHECK(t03_decode_rect(&d14_color_g01_image,
          0u,233u,320u,8u,band,sizeof band)!=0);
    CHECK(t03_decode_rect(&d14_color_g01_image,
          320u,0u,1u,1u,band,sizeof band)!=0);
    CHECK(t03_decode_rect(&d14_color_g01_image,
          0u,0u,0u,8u,band,sizeof band)!=0);
    CHECK(t03_decode_rect(&d14_color_g01_image,
          0u,0u,320u,8u,NULL,sizeof band)!=0);
    printf("D14 G01 PR90 PASS: %u pixels, exact quantized RGB565BE, "
           "30 bands, ROI and rejects\\n",320u*240u);
    return 0;
}
