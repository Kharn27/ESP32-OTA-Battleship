/* D14 COLOR TEST ONLY: A profile, direct RGB565 swatches then
 * PO-approved G01 PR#90 *PNG*, quantized with the pinned T03 encoder.
 * NO SVG/G03 pages. No NVS/touch/CPU/gameplay or product LCD changes.
 * LCD SPI2 20MHz, case10, RGB BIG endian, LEDC PWM 40%, DMA 5120.
 */
#include "d14_color_diag.h"
#include "t03_decode.h"
#include "warship_lcd.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define D14_COLOR_HOLD_MS 3500u
#define G01_SHA256 "ca73f029e80c95c09b3c7defe1437bba3156280b2f2169465ee3f30014343873"
#define G01_SOURCE_BLOB_SHA "7bd321c6ccbfc230802e20010bfa06e71a9099a9"

extern const T03Image d14_color_g01_image; /* build-time generated Flash */
static const char *const TAG = "D14_COLOR";

static void diagnostic_metrics(const char *phase)
{
    WarshipLcdInfo lcd={0};
    (void)warship_lcd_info(&lcd);
    ESP_LOGI(TAG,"D14_COLOR_MEM profile=A phase=%s"
             " heap8=%u largest8=%u stack_margin_bytes=%u"
             " lcd_bands=%u lcd_fault=%u timeouts=%u last_error=%s",
             phase,
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
             (unsigned)uxTaskGetStackHighWaterMark(NULL),
             (unsigned)lcd.transfers,(unsigned)lcd.dma_fault,
             (unsigned)lcd.timeouts,esp_err_to_name(lcd.last_error));
}

static esp_err_t diagnostic_draw(bool g01)
{
    uint8_t *band=warship_lcd_band();
    if(!band) return ESP_ERR_INVALID_STATE;
    const int64_t started=esp_timer_get_time();
    int64_t compose_us=0,lcd_us=0;
    unsigned bands=0u;
    for(unsigned top=0u;top<D14_COLOR_H;top+=D14_COLOR_BAND_ROWS) {
        const int64_t compose_start=esp_timer_get_time();
        const int status=g01 ?
            t03_decode_rect(&d14_color_g01_image,0u,(uint16_t)top,
                            D14_COLOR_W,D14_COLOR_BAND_ROWS,
                            band,WARSHIP_LCD_DMA_BYTES) :
            d14_color_direct_band((uint16_t)top,D14_COLOR_BAND_ROWS,
                                  band,WARSHIP_LCD_DMA_BYTES);
        compose_us+=esp_timer_get_time()-compose_start;
        if(status!=0) {
            ESP_LOGE(TAG,"D14_COLOR_DECODE_FAIL profile=A source=%s row=%u",
                     g01?"G01_PR90_PNG":"DIRECT_RGB565",top);
            return ESP_ERR_INVALID_ARG;
        }
        const int64_t lcd_start=esp_timer_get_time();
        const esp_err_t err=warship_lcd_send(0,(int)top,320,8);
        lcd_us+=esp_timer_get_time()-lcd_start;
        if(err!=ESP_OK) return err;
        ++bands;
    }
    ESP_LOGI(TAG,"D14_COLOR_DRAW profile=A source=%s"
             " bands=%u compose_us=%" PRId64
             " lcd_us=%" PRId64 " total_us=%" PRId64
             " dma_bytes=%u source_png_sha256=%s",
             g01?"G01_PR90_PNG":"DIRECT_RGB565",bands,
             compose_us,lcd_us,esp_timer_get_time()-started,
             (unsigned)WARSHIP_LCD_DMA_BYTES,
             g01?G01_SHA256:"none");
    return ESP_OK;
}

void d14_color_diag_run(void)
{
    ESP_LOGW(TAG,"D14_COLOR_DIAGNOSTIC=ON TEST_ONLY"
             " profile=A only no_nvs=1 no_touch=1 no_gameplay=1"
             " no_flash_writes=1 NO_PRODUCT_COLOR_DECISION=1");
    ESP_LOGI(TAG,"D14_COLOR_SOURCE source=G01_PR90_PNG"
             " approval=PR90 source_main=8fc0a2b4947d9f74023a4d1dc44bd54d680a9847"
             " source_git_blob_sha=%s source_png_sha256=%s"
             " source_png_bytes=144890 quantization=RGB888_to_RGB565BE"
             " pixel_processing=NONE beyond_565_quantization=1"
             " source_svg_used=0",
             G01_SOURCE_BLOB_SHA,G01_SHA256);
    ESP_LOGI(TAG,"D14_COLOR_PIXELS RGB565_BE"
             " BLACK=0000(00:00) WHITE=FFFF(FF:FF)"
             " RED=F800(F8:00) GREEN=07E0(07:E0)"
             " BLUE=001F(00:1F) GRAY=8410(84:10)"
             " YELLOW=FFE0(FF:E0) RAMP_0_to_FFFF=1");
    ESP_LOGI(TAG,"D14_COLOR_PROFILE_REFERENCE"
             " profile=A rgb=1 inversion=0 vendor_gamma=UNCHANGED"
             " case10=1 spi_hz=20000000 dma=5120 pwm_percent=40");
    esp_err_t err=warship_lcd_init();
    if(err==ESP_OK) err=warship_lcd_prepare_console(0x000cu);
    if(err!=ESP_OK) goto failed;
    err=warship_lcd_diag_apply_profile(0u); /* only inversion OFF */
    if(err!=ESP_OK) goto failed;
    diagnostic_metrics("lcd_ready");

    err=diagnostic_draw(false); /* exact pure RGB565 reference, no assets */
    if(err!=ESP_OK) goto failed;
    err=warship_lcd_backlight(true);
    if(err!=ESP_OK) goto failed;
    diagnostic_metrics("DIRECT_RGB565");
    ESP_LOGW(TAG,"D14_COLOR_STEP profile=A source=DIRECT_RGB565"
             " hold_ms=%u next=G01_PR90_PNG",
             (unsigned)D14_COLOR_HOLD_MS);
    vTaskDelay(pdMS_TO_TICKS(D14_COLOR_HOLD_MS));

    err=diagnostic_draw(true); /* exact PNG after 565 quantization */
    if(err!=ESP_OK) goto failed;
    diagnostic_metrics("G01_PR90_PNG");
    ESP_LOGW(TAG,"D14_COLOR_DONE profile=A source=G01_PR90_PNG"
             " G01_PNG_APPROVED=PR90 no_gameplay=1"
             " stays_visible_until_reset=1");
    for(;;) vTaskDelay(pdMS_TO_TICKS(1000));

failed:
    ESP_LOGE(TAG,"D14_COLOR_ABORT profile=A err=%s"
             " LCD_STOP_NO_RETRY=1",esp_err_to_name(err));
    const esp_err_t off=warship_lcd_backlight(false);
    ESP_LOGI(TAG,"D14_COLOR_BL_FORCE_OFF_RESULT err=%s",
             esp_err_to_name(off));
    diagnostic_metrics("fault");
}
