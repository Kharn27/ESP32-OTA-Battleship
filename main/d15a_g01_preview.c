/* D15-A #112: isolated G01 visual test firmware, NO production HOME.
 * Links only four approved CYD assets, composes 30x320x8 DMA bands in
 * Flash RGB565BE + alpha8, and reuses the exact D14 LCD+PWM40 profile.
 * No touch SPI3, no navigation, no game state and NO NVS access.
 */
#include "d15a_g01.h"
#include "warship_lcd.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *const TAG = "D15A_G01";

void app_main(void)
{
    ESP_LOGW(TAG, "D15A_G01_TEST_ONLY=ON NO_GAME=1 NO_NVS=1 NO_TOUCH=1"
             " NO_NAVIGATION=1 LCD_PROFILE=D14_DOOM_NATIVE_LANDSCAPE");
    ESP_LOGI(TAG, "G01_ASSET_LAYERS count=4 order=wallpaper,logo,jouer,palmares"
             " origin=assets/manifest.json flash_raw565be=1 alpha8=1");
    esp_err_t err = warship_lcd_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD_INIT_FAIL err=%s", esp_err_to_name(err));
        return;
    }
    err = warship_lcd_prepare_console(0x0000u);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD_PROFILE_FAIL err=%s", esp_err_to_name(err));
        return;
    }
    for (uint16_t y = 0u; y < D15A_H; y += D15A_BAND_ROWS) {
        uint8_t *const band = warship_lcd_band();
        if (band == NULL ||
            d15a_compose_band(y, D15A_BAND_ROWS,
                              band, D15A_BAND_BYTES) != 0) {
            ESP_LOGE(TAG, "G01_COMPOSE_FAIL row=%u", (unsigned)y);
            return; /* fail closed: LEDC stays OFF */
        }
        err = warship_lcd_send(0, (int)y, D15A_W, D15A_BAND_ROWS);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "G01_LCD_SEND_FAIL row=%u err=%s",
                     (unsigned)y, esp_err_to_name(err));
            return; /* LCD transport makes backlight fail-closed */
        }
    }
    err = warship_lcd_backlight(true);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "G01_BACKLIGHT_FAIL err=%s", esp_err_to_name(err));
        return;
    }
    WarshipLcdInfo info;
    if (!warship_lcd_info(&info)) {
        ESP_LOGE(TAG, "G01_INFO_FAIL");
        (void)warship_lcd_backlight(false);
        return;
    }
    size_t flash_bytes = 0u;
    for (unsigned i = 0u; i < D15A_IMAGE_COUNT; ++i)
        flash_bytes += d15a_assets[i].bytes;
    ESP_LOGI(TAG, "G01_DRAW_PASS screen=320x240 bands=30"
             " logical_layers=4 flash_pixel_bytes=%u"
             " dma_allocated_bytes=%u transfers=%u",
             (unsigned)flash_bytes,(unsigned)D15A_BAND_BYTES,
             (unsigned)info.transfers);
    ESP_LOGI(TAG, "G01_MEM heap8=%u largest8=%u stack_min_free_bytes=%u",
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
             (unsigned)uxTaskGetStackHighWaterMark(NULL));
    ESP_LOGI(TAG, "G01_COUNTERS lcd_fault=%u timeouts=%u"
             " last_error=%s touch_spi_errors=0(not_initialized)",
             (unsigned)info.dma_fault,(unsigned)info.timeouts,
             esp_err_to_name(info.last_error));
    ESP_LOGW(TAG, "PREVIEW_STATIC=1 buttons_are_pixels_only=1"
             " D15_PRODUCT_NOT_IMPLEMENTED=1");

    /* Keep only the isolated visual preview alive; never open NVS,
     * start D09 gameplay or attempt any touch action. */
    for (;;) vTaskDelay(pdMS_TO_TICKS(5000));
}
