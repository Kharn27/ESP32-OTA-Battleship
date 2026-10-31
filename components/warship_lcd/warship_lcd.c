/*
 * T02 CYD LCD transport (independent WARSHIP implementation).
 * Based on the hardware behavior documented for the PO's already-tested
 * Locomotive BASIC CYD console; no original GPLv3 source or header is copied.
 * See docs/T02_CYD_PROBE.md for a source/protocol comparison.
 */
#include "warship_lcd.h"

#include <stddef.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_lcd_ili9341.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define LCD_SPI SPI2_HOST
#define LCD_MOSI GPIO_NUM_13
#define LCD_SCK GPIO_NUM_14
#define LCD_CS GPIO_NUM_15
#define LCD_DC GPIO_NUM_2
#define LCD_LIGHT GPIO_NUM_21
#define LCD_CLOCK 20000000

static const char *const TAG = "WARSHIP_LCD";
static esp_lcd_panel_handle_t panel;
static esp_lcd_panel_io_handle_t panel_io;
static SemaphoreHandle_t completed;
static uint8_t *stripe;
static bool owns_bus, initialized, prepared, fault;
static uint32_t transfers, timeouts;
static esp_err_t last_error = ESP_OK;

static bool IRAM_ATTR on_transfer_done(esp_lcd_panel_io_handle_t sender,
                                       esp_lcd_panel_io_event_data_t *event,
                                       void *opaque)
{
    (void)sender;
    (void)event;
    BaseType_t wake = pdFALSE;
    xSemaphoreGiveFromISR((SemaphoreHandle_t)opaque, &wake);
    return wake == pdTRUE;
}

static esp_err_t sticky_error(esp_err_t reason)
{
    last_error = reason;
    fault = true;
    (void)gpio_set_level(LCD_LIGHT, 0);
    ESP_LOGE(TAG, "FAIL err=%s dma_buffer_retained=1 no_retry=1",
             esp_err_to_name(reason));
    return reason;
}

/* Serialized transfer to an explicit LCD window. The special native-wide
 * 320x240 + MADCTL.MV=0 layout is intentional for this specific CYD.
 * Width/height are always bounded, regardless of panel address semantics.
 */
static esp_err_t transfer(int x, int y, int w, int h, int max_w, int max_h)
{
    if (!panel || !completed || !stripe || fault) return ESP_ERR_INVALID_STATE;
    if (x < 0 || y < 0 || w <= 0 || h <= 0 ||
        x >= max_w || y >= max_h || w > max_w - x || h > max_h - y ||
        (size_t)w * (size_t)h > WARSHIP_LCD_DMA_BYTES / 2u)
        return ESP_ERR_INVALID_ARG;

    (void)xSemaphoreTake(completed, 0);
    esp_err_t err = esp_lcd_panel_draw_bitmap(panel, x, y, x + w, y + h, stripe);
    if (err != ESP_OK) return sticky_error(err);
    if (xSemaphoreTake(completed, pdMS_TO_TICKS(2000)) != pdTRUE) {
        ++timeouts;
        return sticky_error(ESP_ERR_TIMEOUT);
    }
    ++transfers;
    last_error = ESP_OK;
    return ESP_OK;
}

static void cleanup_before_dma(void)
{
    if (panel) { (void)esp_lcd_panel_del(panel); panel = NULL; }
    if (panel_io) { (void)esp_lcd_panel_io_del(panel_io); panel_io = NULL; }
    if (owns_bus) { (void)spi_bus_free(LCD_SPI); owns_bus = false; }
    if (completed) { vSemaphoreDelete(completed); completed = NULL; }
    if (stripe) { heap_caps_free(stripe); stripe = NULL; }
    (void)gpio_set_level(LCD_LIGHT, 0);
}

esp_err_t warship_lcd_init(void)
{
    if (initialized || fault || panel || panel_io || owns_bus ||
        completed || stripe) return ESP_ERR_INVALID_STATE;

    const gpio_config_t light = {
        .pin_bit_mask = 1ULL << LCD_LIGHT,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&light);
    if (err != ESP_OK) return err;
    err = gpio_set_level(LCD_LIGHT, 0);
    if (err != ESP_OK) return err;

    stripe = heap_caps_malloc(WARSHIP_LCD_DMA_BYTES,
                              MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    if (stripe == NULL) { err = ESP_ERR_NO_MEM; goto fail; }
    completed = xSemaphoreCreateBinary();
    if (completed == NULL) { err = ESP_ERR_NO_MEM; goto fail; }

    const spi_bus_config_t bus = {
        .sclk_io_num = LCD_SCK, .mosi_io_num = LCD_MOSI,
        .miso_io_num = -1, .quadwp_io_num = -1, .quadhd_io_num = -1,
        .max_transfer_sz = WARSHIP_LCD_DMA_BYTES + 16,
    };
    err = spi_bus_initialize(LCD_SPI, &bus, SPI_DMA_CH_AUTO);
    if (err != ESP_OK) goto fail;
    owns_bus = true;

    const esp_lcd_panel_io_spi_config_t io = {
        .cs_gpio_num = LCD_CS, .dc_gpio_num = LCD_DC,
        .spi_mode = 0, .pclk_hz = LCD_CLOCK,
        .trans_queue_depth = 2, .lcd_cmd_bits = 8, .lcd_param_bits = 8,
    };
    err = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI,
                                    &io, &panel_io);
    if (err != ESP_OK) goto fail;
    const esp_lcd_panel_io_callbacks_t cb = {
        .on_color_trans_done = on_transfer_done,
    };
    err = esp_lcd_panel_io_register_event_callbacks(panel_io, &cb, completed);
    if (err != ESP_OK) goto fail;

    const esp_lcd_panel_dev_config_t lcd = {
        .reset_gpio_num = -1,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
        .bits_per_pixel = 16,
    };
    err = esp_lcd_new_panel_ili9341(panel_io, &lcd, &panel);
    if (err != ESP_OK) goto fail;
    err = esp_lcd_panel_reset(panel);
    if (err != ESP_OK) goto fail;
    err = esp_lcd_panel_init(panel);
    if (err != ESP_OK) goto fail;
    err = esp_lcd_panel_disp_on_off(panel, true);
    if (err != ESP_OK) goto fail;

    initialized = true;
    ESP_LOGI(TAG, "INIT_PASS LCD=ILI9341 SPI2=13,14,15,2 BL=21 clock_hz=%u dma_bytes=%u",
             (unsigned)LCD_CLOCK, (unsigned)WARSHIP_LCD_DMA_BYTES);
    return ESP_OK;

fail:
    last_error = err;
    /* No DMA has been submitted in init(). Safe to release allocations. */
    cleanup_before_dma();
    return err;
}

esp_err_t warship_lcd_prepare_console(uint16_t clear_rgb565)
{
    if (!initialized || fault) return ESP_ERR_INVALID_STATE;
    if (prepared) {
        ESP_LOGI(TAG, "PROFILE_REUSED case=10 madctl_unchanged=1 no_clear=1");
        return ESP_OK;
    }

    /* PO-proven console sequence: MV=0 + MX=MY=0, then a 320x240
     * wide clear via CASET/RASET/RAMWR. This is nonstandard for a nominal
     * 240-column controller, but the SAME sequence is used on the PO's CYD.
     * Do not silently change to a generic MV=1 path.
     */
    esp_err_t err = esp_lcd_panel_swap_xy(panel, false);
    if (err != ESP_OK) return sticky_error(err);
    err = esp_lcd_panel_mirror(panel, false, false);
    if (err != ESP_OK) return sticky_error(err);

    ESP_LOGI(TAG, "PROFILE_BEGIN case=10 native_wide=1 mv=0 clear_window=320x240");
    for (unsigned row = 0u; row < WARSHIP_LCD_HEIGHT; row += WARSHIP_LCD_BAND_ROWS) {
        for (unsigned i = 0u; i < WARSHIP_LCD_WIDTH * WARSHIP_LCD_BAND_ROWS; ++i) {
            stripe[2u * i] = (uint8_t)(clear_rgb565 >> 8);
            stripe[2u * i + 1u] = (uint8_t)clear_rgb565;
        }
        err = transfer(0, (int)row, (int)WARSHIP_LCD_WIDTH,
                       (int)WARSHIP_LCD_BAND_ROWS,
                       (int)WARSHIP_LCD_WIDTH, (int)WARSHIP_LCD_HEIGHT);
        if (err != ESP_OK) return err;
    }

    /* Preserve the reference's second MADCTL sequence, even though
     * MV is already clear. Console case 10 => MV=0, MX=0, MY=1.
     */
    err = esp_lcd_panel_swap_xy(panel, false);
    if (err != ESP_OK) return sticky_error(err);
    err = esp_lcd_panel_mirror(panel, false, true);
    if (err != ESP_OK) return sticky_error(err);
    prepared = true;
    ESP_LOGI(TAG, "PROFILE_APPLIED case=10 mv=0 mx=0 my=1 window=320x240 clear_bands=30");
    return ESP_OK;
}

uint8_t *warship_lcd_band(void)
{
    return initialized && prepared && !fault ? stripe : NULL;
}

esp_err_t warship_lcd_send(int x, int y, int width, int height)
{
    if (!prepared) return ESP_ERR_INVALID_STATE;
    return transfer(x, y, width, height, (int)WARSHIP_LCD_WIDTH,
                    (int)WARSHIP_LCD_HEIGHT);
}

esp_err_t warship_lcd_backlight(bool on)
{
    if (!prepared || fault) return ESP_ERR_INVALID_STATE;
    const esp_err_t err = gpio_set_level(LCD_LIGHT, on ? 1 : 0);
    if (err != ESP_OK) return sticky_error(err);
    last_error = ESP_OK;
    return ESP_OK;
}

bool warship_lcd_info(WarshipLcdInfo *out)
{
    if (out == NULL) return false;
    out->initialized = initialized;
    out->profile_applied = prepared;
    out->dma_fault = fault;
    out->transfers = transfers;
    out->timeouts = timeouts;
    out->last_error = last_error;
    return true;
}
