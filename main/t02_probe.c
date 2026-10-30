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

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_chip_info.h"
#include "esp_err.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_lcd_ili9341.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define LCD_BUS SPI2_HOST
#define LCD_PIN_MOSI GPIO_NUM_13
#define LCD_PIN_SCLK GPIO_NUM_14
#define LCD_PIN_CS GPIO_NUM_15
#define LCD_PIN_DC GPIO_NUM_2
#define LCD_PIN_BL GPIO_NUM_21
#define LCD_CLOCK_HZ 20000000
#define LCD_W 320
#define LCD_H 240
#define LCD_TILE_ROWS 8
#define LCD_DMA_BYTES (LCD_W * LCD_TILE_ROWS * 2)

/* Candidate CYD XPT2046 wire mapping, not established by PENIRQ test.
 * SPI3 is physically separate from the LCD SPI2 bus.
 */
#define TOUCH_BUS SPI3_HOST
#define TOUCH_PIN_MOSI GPIO_NUM_32
#define TOUCH_PIN_MISO GPIO_NUM_39
#define TOUCH_PIN_SCLK GPIO_NUM_25
#define TOUCH_PIN_CS GPIO_NUM_33
#define TOUCH_PIN_IRQ GPIO_NUM_36
#define TOUCH_CLOCK_HZ 1000000

static const char *TAG = "T02";
static esp_lcd_panel_handle_t panel;
static esp_lcd_panel_io_handle_t panel_io;
static SemaphoreHandle_t dma_done;
static uint8_t *dma_buf;
static spi_device_handle_t touch_dev;
static bool lcd_fault;
static uint32_t bands_sent, dma_timeouts, samples_ok, samples_invalid;

/* Do NOT mistake the bounds below for calibration. They encode a linear
 * diagnostic display of the 12-bit ADC range, without confirmed orientation.
 */
static const T02TouchMap diagnostic_map = {
    .x_at_left = 0u, .x_at_right = 4095u,
    .y_at_top = 0u, .y_at_bottom = 4095u,
    .width = LCD_W, .height = LCD_H,
};

static bool IRAM_ATTR lcd_complete(esp_lcd_panel_io_handle_t io,
                                   esp_lcd_panel_io_event_data_t *ev,
                                   void *ctx)
{
    (void)io;
    (void)ev;
    BaseType_t awakened = pdFALSE;
    xSemaphoreGiveFromISR((SemaphoreHandle_t)ctx, &awakened);
    return awakened == pdTRUE;
}

static void snapshot(const char *phase)
{
    ESP_LOGI(TAG, "MEM phase=%s heap8=%u largest8=%u min8=%u dma_free=%u dma_largest=%u stack_margin_bytes=%u lcd_dma_bytes=%u",
             phase,
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA),
             (unsigned)uxTaskGetStackHighWaterMark(NULL),
             (unsigned)LCD_DMA_BYTES);
}

static esp_err_t send_band(int x, int y, int width, int height)
{
    if (lcd_fault || !panel || !dma_done) return ESP_ERR_INVALID_STATE;
    if (x < 0 || y < 0 || width <= 0 || height <= 0 ||
        x >= LCD_W || y >= LCD_H ||
        width > LCD_W - x || height > LCD_H - y ||
        (size_t)width * (size_t)height > LCD_DMA_BYTES / 2u) {
        return ESP_ERR_INVALID_ARG;
    }
    (void)xSemaphoreTake(dma_done, 0);
    const esp_err_t err = esp_lcd_panel_draw_bitmap(panel, x, y,
                                                     x + width, y + height,
                                                     dma_buf);
    if (err != ESP_OK) {
        lcd_fault = true;
        (void)gpio_set_level(LCD_PIN_BL, 0);
        ESP_LOGE(TAG, "LCD_FAIL transfer=%s sticky=1 dma_memory_retained=1",
                 esp_err_to_name(err));
        return err;
    }
    if (xSemaphoreTake(dma_done, pdMS_TO_TICKS(2000)) != pdTRUE) {
        ++dma_timeouts;
        lcd_fault = true;
        (void)gpio_set_level(LCD_PIN_BL, 0);
        ESP_LOGE(TAG, "LCD_TIMEOUT sticky=1 dma_memory_retained=1");
        return ESP_ERR_TIMEOUT;
    }
    ++bands_sent;
    return ESP_OK;
}

static esp_err_t lcd_init(void)
{
    const gpio_config_t bl = {
        .pin_bit_mask = 1ULL << LCD_PIN_BL,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&bl);
    if (err != ESP_OK) return err;
    err = gpio_set_level(LCD_PIN_BL, 0);
    if (err != ESP_OK) return err;
    dma_buf = heap_caps_malloc(LCD_DMA_BYTES,
                               MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    if (!dma_buf) return ESP_ERR_NO_MEM;
    dma_done = xSemaphoreCreateBinary();
    if (!dma_done) return ESP_ERR_NO_MEM;

    spi_bus_config_t bus = {
        .mosi_io_num = LCD_PIN_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = LCD_PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_DMA_BYTES + 16,
    };
    err = spi_bus_initialize(LCD_BUS, &bus, SPI_DMA_CH_AUTO);
    if (err != ESP_OK) return err;
    const esp_lcd_panel_io_spi_config_t io = {
        .cs_gpio_num = LCD_PIN_CS,
        .dc_gpio_num = LCD_PIN_DC,
        .spi_mode = 0,
        .pclk_hz = LCD_CLOCK_HZ,
        .trans_queue_depth = 2,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    err = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_BUS,
                                    &io, &panel_io);
    if (err != ESP_OK) return err;
    const esp_lcd_panel_io_callbacks_t callbacks = {
        .on_color_trans_done = lcd_complete,
    };
    err = esp_lcd_panel_io_register_event_callbacks(panel_io, &callbacks,
                                                     dma_done);
    if (err != ESP_OK) return err;
    const esp_lcd_panel_dev_config_t config = {
        .reset_gpio_num = -1, /* NO GPIO12/strap or guessed reset. */
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
        .bits_per_pixel = 16,
    };
    err = esp_lcd_new_panel_ili9341(panel_io, &config, &panel);
    if (err != ESP_OK) return err;
    err = esp_lcd_panel_reset(panel);
    if (err != ESP_OK) return err;
    err = esp_lcd_panel_init(panel);
    if (err != ESP_OK) return err;
    err = esp_lcd_panel_disp_on_off(panel, true);
    if (err != ESP_OK) return err;

    /* Known reference "case 9": native-wide addressing (swap_xy=false,
     * mirror_x=true, mirror_y=false). First clear native 240x320 GRAM,
     * then set this case without switching orientations during T02.
     */
    err = esp_lcd_panel_swap_xy(panel, false);
    if (err != ESP_OK) return err;
    err = esp_lcd_panel_mirror(panel, false, false);
    if (err != ESP_OK) return err;
    ESP_LOGI(TAG, "LCD_INIT native=240x320 wanted=320x240 case=9 clock_hz=%u pins=13,14,15,2,21 reset_gpio=-1",
             (unsigned)LCD_CLOCK_HZ);
    for (int row = 0; row < 320; row += LCD_TILE_ROWS) {
        memset(dma_buf, 0, 240 * LCD_TILE_ROWS * 2);
        /* Native 240x320 clear cannot use landscape send_band bounds. */
        (void)xSemaphoreTake(dma_done, 0);
        err = esp_lcd_panel_draw_bitmap(panel, 0, row, 240,
                                        row + LCD_TILE_ROWS, dma_buf);
        if (err != ESP_OK ||
            xSemaphoreTake(dma_done, pdMS_TO_TICKS(2000)) != pdTRUE) {
            lcd_fault = true;
            (void)gpio_set_level(LCD_PIN_BL, 0);
            ESP_LOGE(TAG, "LCD_NATIVE_CLEAR_FAIL err=%s", esp_err_to_name(err));
            return err != ESP_OK ? err : ESP_ERR_TIMEOUT;
        }
        ++bands_sent;
    }
    err = esp_lcd_panel_swap_xy(panel, false);
    if (err != ESP_OK) return err;
    err = esp_lcd_panel_mirror(panel, true, false);
    if (err == ESP_OK) ESP_LOGI(TAG, "LCD_ORIENTATION candidate_case=9 320x240 check_physical=1");
    return err;
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

static esp_err_t draw_rect(int x, int y, int width, int height, unsigned phase)
{
    if (width <= 0 || width > LCD_W || height <= 0 ||
        (size_t)width * LCD_TILE_ROWS * 2u > LCD_DMA_BYTES)
        return ESP_ERR_INVALID_ARG;
    const int64_t start = esp_timer_get_time();
    for (int top = 0; top < height; top += LCD_TILE_ROWS) {
        const int tile_h = height - top < LCD_TILE_ROWS ? height - top : LCD_TILE_ROWS;
        for (int r = 0; r < tile_h; ++r) {
            for (int col = 0; col < width; ++col) {
                const uint16_t c = pixel(x + col, y + top + r, phase);
                const size_t offset = 2u * ((size_t)r * (size_t)width + (size_t)col);
                dma_buf[offset] = (uint8_t)(c >> 8);
                dma_buf[offset + 1u] = (uint8_t)c;
            }
        }
        esp_err_t err = send_band(x, y + top, width, tile_h);
        if (err != ESP_OK) return err;
    }
    ESP_LOGI(TAG, "DRAW region=%dx%d at=%d,%d phase=%u time_us=%" PRId64 " transfers=%u",
             width, height, x, y, phase, esp_timer_get_time() - start,
             (unsigned)bands_sent);
    return ESP_OK;
}

/* Dedicated SPI3 and CS33; NOT the LCD SPI2 bus. */
static esp_err_t touch_init(void)
{
    const gpio_config_t irq = {
        .pin_bit_mask = 1ULL << TOUCH_PIN_IRQ,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE, /* GPIO36 input-only, no pulls. */
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&irq);
    if (err != ESP_OK) return err;
    spi_bus_config_t bus = {
        .mosi_io_num = TOUCH_PIN_MOSI,
        .miso_io_num = TOUCH_PIN_MISO,
        .sclk_io_num = TOUCH_PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 3,
    };
    err = spi_bus_initialize(TOUCH_BUS, &bus, SPI_DMA_DISABLED);
    if (err != ESP_OK) return err;
    spi_device_interface_config_t device = {
        .clock_speed_hz = TOUCH_CLOCK_HZ,
        .mode = 0,
        .spics_io_num = TOUCH_PIN_CS,
        .queue_size = 1,
    };
    err = spi_bus_add_device(TOUCH_BUS, &device, &touch_dev);
    ESP_LOGW(TAG, "TOUCH_SPI_CANDIDATE bus=SPI3 MOSI=32 MISO=39 SCLK=25 CS=33 IRQ=36 clock=%d raw_xy_not_verified=1 err=%s",
             TOUCH_CLOCK_HZ, esp_err_to_name(err));
    return err;
}

static esp_err_t xpt_read(uint8_t command, uint16_t *out)
{
    if (touch_dev == NULL || out == NULL) return ESP_ERR_INVALID_STATE;
    spi_transaction_t tx = {
        .length = 24,
        .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
    };
    tx.tx_data[0] = command;
    tx.tx_data[1] = 0;
    tx.tx_data[2] = 0;
    esp_err_t err = spi_device_transmit(touch_dev, &tx);
    if (err != ESP_OK) return err;
    const uint16_t packed = ((uint16_t)tx.rx_data[1] << 8) | tx.rx_data[2];
    *out = (packed >> 3) & 0x0fffu;
    return ESP_OK;
}

static uint16_t median3(uint16_t a, uint16_t b, uint16_t c)
{
    if (a > b) { uint16_t t = a; a = b; b = t; }
    if (b > c) { uint16_t t = b; b = c; c = t; }
    if (a > b) { uint16_t t = a; a = b; b = t; }
    return b;
}

static void touch_sample(void)
{
    uint16_t xs[3], ys[3];
    /* XPT2046 command 0xD0 X; 0x90 Y (12-bit single-ended), PD=00
     * returns low power/IRQ mode after every transaction. Not a
     * pressure measurement; PENIRQ controls whether we request ADC data.
     */
    for (unsigned i = 0; i < 3u; ++i) {
        if (xpt_read(0xd0u, &xs[i]) != ESP_OK ||
            xpt_read(0x90u, &ys[i]) != ESP_OK) {
            ++samples_invalid;
            ESP_LOGE(TAG, "TOUCH_SPI_ERROR sample=%u", i);
            return;
        }
    }
    const uint16_t rx = median3(xs[0], xs[1], xs[2]);
    const uint16_t ry = median3(ys[0], ys[1], ys[2]);
    uint16_t px = 0, py = 0;
    const int mapped = t02_map_point(&diagnostic_map, rx, ry, &px, &py);
    const bool saturating = rx == 0u || ry == 0u || rx == 4095u || ry == 4095u;
    ++samples_ok;
    ESP_LOGI(TAG, "TOUCH_RAW sample=%u penirq=LOW x=%u y=%u x_triplet=%u,%u,%u y_triplet=%u,%u,%u logical_provisional=%u,%u mapped=%d saturation=%u",
             (unsigned)samples_ok, (unsigned)rx, (unsigned)ry,
             (unsigned)xs[0], (unsigned)xs[1], (unsigned)xs[2],
             (unsigned)ys[0], (unsigned)ys[1], (unsigned)ys[2],
             (unsigned)px, (unsigned)py, mapped, (unsigned)saturating);
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
    esp_err_t err = lcd_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD_INIT_FAIL err=%s; no framebuffer or touch initialized", esp_err_to_name(err));
        snapshot("lcd_fail");
        return;
    }
    snapshot("lcd_initialized");
    err = draw_rect(0, 0, LCD_W, LCD_H, 0u);
    if (err != ESP_OK) {
        snapshot("draw_fail");
        return; /* DMA buffer kept pinned if transfer status is uncertain. */
    }
    err = gpio_set_level(LCD_PIN_BL, 1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LCD_BACKLIGHT_FAIL err=%s", esp_err_to_name(err));
        return;
    }
    snapshot("screen_visible");
    const bool have_touch = touch_init() == ESP_OK;
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
            const bool pressed = gpio_get_level(TOUCH_PIN_IRQ) == 0;
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
            ESP_LOGI(TAG, "COUNTERS bands=%u dma_timeouts=%u touch_samples=%u touch_failures=%u",
                     (unsigned)bands_sent, (unsigned)dma_timeouts,
                     (unsigned)samples_ok, (unsigned)samples_invalid);
            last_mem = now;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
