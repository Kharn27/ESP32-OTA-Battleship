/*
 * T02 standalone CYD hardware probe. Independent implementation using
 * Espressif esp_lcd and SPI master APIs; NO code copied from
 * ESP32-Locomotive-Basic (a GPLv3 repository).
 *
 * LCD pins originate from a PO-validated hardware reference.
 * Touch SPI pins are documentary CANDIDATES, NOT yet hardware verified.
 * Raw touch values, diagnostic mapping and memory logs are provisional
 * until validated on the actual CYD running this exact firmware SHA.
 */
#include "t02_probe.h"
#include "t02_touch_map.h"
#include "warship_lcd.h"
#include "warship_touch.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_chip_info.h"
#include "esp_err.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LCD_W WARSHIP_LCD_WIDTH
#define LCD_H WARSHIP_LCD_HEIGHT
#define LCD_TILE_ROWS WARSHIP_LCD_BAND_ROWS
#define LCD_DMA_BYTES WARSHIP_LCD_DMA_BYTES

#define TOUCH_PIN_IRQ GPIO_NUM_36
static const char *TAG = "T02";
static uint32_t samples_ok, samples_invalid;

/* Uncalibrated diagnostic full ADC range, unchanged by D09. */
static const T02TouchMap diagnostic_map = {
    .x_at_left=0u, .x_at_right=4095u,
    .y_at_top=0u, .y_at_bottom=4095u,
    .width=LCD_W, .height=LCD_H,
};

static void snapshot(const char *phase)
{
    WarshipLcdInfo info;
    (void)warship_lcd_info(&info);
    ESP_LOGI(TAG, "MEM phase=%s heap8=%u largest8=%u min8=%u dma_free=%u dma_largest=%u stack_margin_bytes=%u lcd_dma_bytes=%u lcd_transfers=%u dma_timeouts=%u dma_fault=%u",
             phase,
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA),
             (unsigned)uxTaskGetStackHighWaterMark(NULL),
             (unsigned)LCD_DMA_BYTES,
             (unsigned)info.transfers, (unsigned)info.timeouts,
             (unsigned)info.dma_fault);
}

static uint16_t pixel(int x, int y, unsigned phase)
{
    /* Four distinct corners, center, screen perimeter and RGB bands. */
    if (x < 20 && y < 20) return 0xffffu;         /* top-left white */
    if (x >= LCD_W - 20 && y < 20) return 0x0000u; /* top-right black */
    if (x < 20 && y >= LCD_H - 20) return 0xffe0u; /* bottom-left yellow */
    if (x >= LCD_W - 20 && y >= LCD_H - 20) return 0xf81fu; /* bottom-right magenta */
    if ((x >= 155 && x <= 165 && y >= 111 && y <= 129) ||
        (y >= 115 && y <= 125 && x >= 151 && x <= 169))
        return 0x07ffu; /* cyan center cross */
    if (x < 2 || y < 2 || x >= LCD_W - 2 || y >= LCD_H - 2)
        return 0xffffu;
    if (x >= 136 && x < 184 && y >= 168 && y < 200)
        return (phase & 1u) ? 0x0000u : 0xffffu; /* refreshed subrectangle */
    if (x < 107) return 0xf800u; /* red */
    if (x < 214) return 0x07e0u; /* green */
    return 0x001fu; /* blue */
}

static esp_err_t draw_rect(int x, int y, int width, int height,
                           unsigned phase)
{
    if (x < 0 || y < 0 || width <= 0 || height <= 0 ||
        x >= (int)LCD_W || y >= (int)LCD_H ||
        width > (int)LCD_W - x || height > (int)LCD_H - y ||
        (size_t)width * LCD_TILE_ROWS * 2u > LCD_DMA_BYTES)
        return ESP_ERR_INVALID_ARG;

    uint8_t *const buffer = warship_lcd_band();
    if (buffer == NULL) return ESP_ERR_INVALID_STATE;
    const int64_t start = esp_timer_get_time();
    for (int top = 0; top < height; top += (int)LCD_TILE_ROWS) {
        const int tile_h = height - top < (int)LCD_TILE_ROWS ?
                           height - top : (int)LCD_TILE_ROWS;
        for (int r = 0; r < tile_h; ++r) {
            for (int col = 0; col < width; ++col) {
                const uint16_t color = pixel(x + col, y + top + r, phase);
                const size_t at = 2u * ((size_t)r * (size_t)width + (size_t)col);
                buffer[at] = (uint8_t)(color >> 8);
                buffer[at + 1u] = (uint8_t)color;
            }
        }
        esp_err_t err = warship_lcd_send(x, y + top, width, tile_h);
        if (err != ESP_OK) return err;
    }
    WarshipLcdInfo info;
    (void)warship_lcd_info(&info);
    ESP_LOGI(TAG, "DRAW region=%dx%d at=%d,%d phase=%u time_us=%" PRId64 " transfers=%u",
             width, height, x, y, phase, esp_timer_get_time() - start,
             (unsigned)info.transfers);
    return ESP_OK;
}

static void touch_sample(void)
{
    WarshipTouchSample s;
    const esp_err_t err=warship_touch_sample(&s);
    if (err!=ESP_OK) {
        ++samples_invalid;
        ESP_LOGE(TAG, "TOUCH_SPI_ERROR err=%s", esp_err_to_name(err));
        return;
    }
    uint16_t px=0u,py=0u;
    const int mapped=t02_map_point(&diagnostic_map,s.x,s.y,&px,&py);
    const bool saturated=s.x==0u || s.y==0u ||
                         s.x==4095u || s.y==4095u;
    ++samples_ok;
    ESP_LOGI(TAG,"TOUCH_RAW sample=%u penirq=LOW x=%u y=%u x_triplet=%u,%u,%u y_triplet=%u,%u,%u logical_provisional=%u,%u mapped=%d saturation=%u",
             (unsigned)samples_ok,(unsigned)s.x,(unsigned)s.y,
             (unsigned)s.xs[0],(unsigned)s.xs[1],(unsigned)s.xs[2],
             (unsigned)s.ys[0],(unsigned)s.ys[1],(unsigned)s.ys[2],
             (unsigned)px,(unsigned)py,mapped,(unsigned)saturated);
}

void t02_probe_run(void)
{
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    uint32_t flash_bytes = 0;
    const esp_err_t flash_err = esp_flash_get_size(esp_flash_default_chip,
                                                  &flash_bytes);
    ESP_LOGI(TAG, "BOOT probe=T02 chip_model=%d revision=%u cores=%u flash_idf=%u flash_err=%s psram_not_assumed=1",
             chip.model, (unsigned)chip.revision, (unsigned)chip.cores,
             (unsigned)flash_bytes, esp_err_to_name(flash_err));
    ESP_LOGW(TAG, "This is a DIAGNOSTIC: touch raw, pin mapping and display orientation require PO physical checks");
    snapshot("startup");
    esp_err_t err = warship_lcd_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD_INIT_FAIL err=%s; no framebuffer or touch initialized", esp_err_to_name(err));
        snapshot("lcd_fail");
        return;
    }
    snapshot("lcd_initialized");
    err = warship_lcd_prepare_console(0x0000u);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD_PROFILE_FAIL err=%s", esp_err_to_name(err));
        snapshot("orientation_fail");
        return;
    }
    ESP_LOGI(TAG, "LCD_PROFILE_PASS console_case=10 mv=0 mx=0 my=1");
    snapshot("lcd_profile_applied");
    err = draw_rect(0, 0, (int)LCD_W, (int)LCD_H, 0u);
    if (err != ESP_OK) {
        snapshot("draw_fail");
        return; /* DMA buffer kept pinned if transfer status is uncertain. */
    }
    err = warship_lcd_backlight(true);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD_BACKLIGHT_FAIL err=%s", esp_err_to_name(err));
        return;
    }
    snapshot("screen_visible");
    const bool have_touch = warship_touch_init() == ESP_OK;
    snapshot(have_touch ? "touch_initialized" : "touch_init_fail");
    unsigned phase = 0u;
    int64_t last_draw = esp_timer_get_time();
    int64_t last_mem = last_draw;
    int64_t last_sample = last_draw;
    bool was_pressed = false;
    uint8_t press_confirm = 0u;
    for (;;) {
        const int64_t now = esp_timer_get_time();
        if (have_touch) {
            const bool pressed = warship_touch_pressed();
            if (pressed) {
                if (press_confirm < 3u) ++press_confirm;
            } else {
                press_confirm = 0u;
            }
            const bool debounced = press_confirm >= 2u;
            if (debounced && !was_pressed) {
                ESP_LOGI(TAG, "TOUCH_PRESS irq=36");
            }
            if (!debounced && was_pressed) {
                ESP_LOGI(TAG, "TOUCH_RELEASE irq=36");
            }
            was_pressed = debounced;
            if (debounced && now - last_sample >= 120000) {
                touch_sample();
                last_sample = now;
            }
        }
        if (now - last_draw >= 2000000) {
            ++phase;
            err = draw_rect(136, 168, 48, 32, phase);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "LCD_REFRESH_STOP");
                snapshot("lcd_fault");
                return;
            }
            last_draw = now;
        }
        if (now - last_mem >= 10000000) {
            snapshot("running");
            WarshipLcdInfo lcd;
            (void)warship_lcd_info(&lcd);
            ESP_LOGI(TAG, "COUNTERS bands=%u dma_timeouts=%u lcd_fault=%u touch_samples=%u touch_failures=%u",
                     (unsigned)lcd.transfers, (unsigned)lcd.timeouts,
                     (unsigned)lcd.dma_fault,
                     (unsigned)samples_ok, (unsigned)samples_invalid);
            last_mem = now;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
