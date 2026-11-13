/* #128 TEST_ONLY one-screen CYD 320x240 physical inspection.
 * No NVS init, no touch, no game, no PSRAM, no PNG parser.
 */
#include "portraits01.h"
#include "warship_lcd.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <inttypes.h>
static const char *const TAG="PORTRAITS01_TEST_ONLY";
static void stats(const char *phase)
{
 ESP_LOGI(TAG,
  "PORTRAITS01_MEM phase=%s free8=%u min8=%u largest8=%u stack_hwm_bytes=%u",
  phase,
  (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
  (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT),
  (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
  (unsigned)uxTaskGetStackHighWaterMark(NULL));
}
void app_main(void)
{
 ESP_LOGW(TAG,"PORTRAITS01_TFT_TEST_ONLY=ON source_head=dab0a35a1a891ab4d215a55cd16f2fe095a558df");
 ESP_LOGW(TAG,"PORTRAITS01_CANDIDATE not_an_approved_asset no_PO_artistic_GO");
 ESP_LOGI(TAG,"PORTRAITS01_LAYOUT native=320x240 top_dark=[4,16,312,100] bottom_light=[4,128,312,100]");
 for(unsigned i=0;i<P01_COUNT;++i) {
  const Portrait01Sprite *s=&portrait01_sprites[i];
  const unsigned lx=9u+103u*i;
  ESP_LOGI(TAG,"PORTRAITS01_SOURCE size=%ux%u png_sha256=%s flash_bytes=%u top_xy=%u,%u bottom_xy=%u,%u",
   (unsigned)s->width,(unsigned)s->height,s->source_sha256,
   (unsigned)s->bytes,(unsigned)(lx+(94u-s->width)/2u),
   (unsigned)(110u-s->height),(unsigned)(lx+(94u-s->width)/2u),
   (unsigned)(220u-s->height));
 }
 stats("before_lcd");
 esp_err_t err=warship_lcd_init();
 if(err==ESP_OK)err=warship_lcd_prepare_console(0x0000u);
 if(err!=ESP_OK)goto fail;
 stats("after_init");
 WarshipLcdInfo before={0},after={0};
 if(!warship_lcd_info(&before)){err=ESP_FAIL;goto fail;}
 int64_t compose_us=0,send_us=0;
 const int64_t start_us=esp_timer_get_time();
 for(unsigned y=0;y<P01_H;y+=P01_BAND_ROWS) {
  uint8_t *const dma=warship_lcd_band();
  if(!dma){err=ESP_ERR_NO_MEM;goto fail;}
  const int64_t t0=esp_timer_get_time();
  if(portrait01_compose_band(y,P01_BAND_ROWS,dma,P01_BAND_BYTES)!=0){
   err=ESP_FAIL;goto fail;
  }
  const int64_t t1=esp_timer_get_time();
  compose_us+=t1-t0;
  err=warship_lcd_send(0,(int)y,(int)P01_W,(int)P01_BAND_ROWS);
  send_us+=esp_timer_get_time()-t1;
  if(err!=ESP_OK)goto fail;
 }
 if(!warship_lcd_info(&after)){err=ESP_FAIL;goto fail;}
 ESP_LOGI(TAG,"PORTRAITS01_RENDER bands=%u pixel_bytes=%u compose_us=%" PRId64
          " lcd_send_us=%" PRId64 " total_us=%" PRId64,
          (unsigned)(P01_H/P01_BAND_ROWS),(unsigned)(P01_W*P01_H*2u),
          compose_us,send_us,esp_timer_get_time()-start_us);
 ESP_LOGI(TAG,"PORTRAITS01_LCD delta_transfers=%" PRIu32
          " delta_timeouts=%" PRIu32 " dma_fault=%u last_err=%s",
          after.transfers-before.transfers,after.timeouts-before.timeouts,
          (unsigned)after.dma_fault,esp_err_to_name(after.last_error));
 err=warship_lcd_backlight(true);
 if(err!=ESP_OK)goto fail;
 stats("after_draw");
 ESP_LOGW(TAG,"PORTRAITS01_READY inspect_both_backgrounds no_touch no_nvs no_game");
 return;
fail:
 ESP_LOGE(TAG,"PORTRAITS01_FAIL error=%s",esp_err_to_name(err));
 (void)warship_lcd_backlight(false);
 stats("failed");
}
