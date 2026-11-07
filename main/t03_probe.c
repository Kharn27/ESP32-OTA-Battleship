/* T03 OPT-IN diagnostic; caller has already initialized SPI2 LCD, case 10.
 * No second bus/driver/DMA/partition init. D09 normal screen is drawn
 * immediately afterwards. Run only before the touch/gameplay loop.
 */
#include "t03_probe.h"
#include "t03_decode.h"
#include "warship_lcd.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern const uint8_t t03_g03_raw565[153600];
extern const uint8_t t03_g03_rle[];
extern const uint32_t t03_g03_row_offsets[241];
extern const size_t t03_g03_rle_size;

static const char *const TAG = "T03";

static esp_err_t show_image(const T03Image *image, const char *format)
{
    uint8_t *const dma = warship_lcd_band();
    if (!dma || !image) return ESP_ERR_INVALID_STATE;
    int64_t decode_us = 0, lcd_us = 0;
    const int64_t before = esp_timer_get_time();
    unsigned bands = 0u;
    for (uint16_t y = 0u; y < 240u; y += WARSHIP_LCD_BAND_ROWS) {
        const int64_t start = esp_timer_get_time();
        if (t03_decode_rect(image, 0u, y, 320u, 8u, dma,
                            WARSHIP_LCD_DMA_BYTES) != 0) {
            ESP_LOGE(TAG, "DECODE_FAIL format=%s band_y=%u",
                     format, (unsigned)y);
            return ESP_FAIL;
        }
        decode_us += esp_timer_get_time() - start;
        const int64_t sending = esp_timer_get_time();
        const esp_err_t err = warship_lcd_send(0, y, 320, 8);
        lcd_us += esp_timer_get_time() - sending;
        if (err != ESP_OK) {
            ESP_LOGE(TAG,"LCD_FAIL format=%s band_y=%u err=%s",
                     format,(unsigned)y,esp_err_to_name(err));
            return err; /* Never overwrite an in-flight/failed DMA band. */
        }
        ++bands;
    }
    WarshipLcdInfo info={0};
    (void)warship_lcd_info(&info);
    ESP_LOGI(TAG, "ASSET format=%s bands=%u decode_us=%" PRId64
             " lcd_us=%" PRId64 " total_us=%" PRId64
             " heap8=%u largest8=%u stack_margin_bytes=%u"
             " lcd_transfers=%u timeouts=%u fault=%u",
             format,bands,decode_us,lcd_us,esp_timer_get_time()-before,
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
             (unsigned)uxTaskGetStackHighWaterMark(NULL),
             (unsigned)info.transfers,(unsigned)info.timeouts,
             (unsigned)info.dma_fault);
    return ESP_OK;
}

esp_err_t t03_probe_compare(void)
{
    const T03Image raw = {t03_g03_raw565,153600u,NULL,0u,
                          320u,240u,T03_RAW565BE};
    const T03Image rle = {t03_g03_rle,t03_g03_rle_size,
                          t03_g03_row_offsets,241u,
                          320u,240u,T03_ROW_RLE565BE};
    ESP_LOGW(TAG,"DIAGNOSTIC_ONLY G03 is PC preview; not final product UI");
    esp_err_t err = show_image(&raw,"raw565be");
    if (err != ESP_OK) return err;
    return show_image(&rle,"row-rle565be");
}
