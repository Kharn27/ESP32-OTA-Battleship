/*
 * T02 CYD LCD transport (independent WARSHIP implementation).
 * Based on the hardware behavior documented for the PO's already-tested
 * Locomotive BASIC CYD console; no original GPLv3 source or header is copied.
 * See docs/T02_CYD_PROBE.md for a source/protocol comparison.
 */
#include "warship_lcd.h"

#include <stddef.h>
#include <string.h>

#include "warship_lcd_light.h"
#include "warship_panel_profile.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_lcd_ili9341.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

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
    const esp_err_t light_err = warship_lcd_light_force_off();
    if (light_err != ESP_OK)
        ESP_LOGE(TAG, "BACKLIGHT_FORCE_OFF_FAILED err=%s",
                 esp_err_to_name(light_err));
    ESP_LOGE(TAG, "FAIL err=%s dma_buffer_retained=1 no_retry=1",
             esp_err_to_name(reason));
    return reason;
}

/* Serialized native 320x240 landscape transfer (MADCTL.MV=1). */
static esp_err_t transfer(int x, int y, int w, int h, int max_w, int max_h)
{
    if (!panel || !completed || !stripe || fault) return ESP_ERR_INVALID_STATE;
    if (x < 0 || y < 0 || w <= 0 || h <= 0 ||
        x >= max_w || y >= max_h || w > max_w - x || h > max_h - y ||
        (size_t)w * (size_t)h > WARSHIP_LCD_DMA_BYTES / 2u)
        return ESP_ERR_INVALID_ARG;

    WarshipPanelWindow window;
    if(warship_panel_window((uint16_t)x,(uint16_t)y,
                            (uint16_t)w,(uint16_t)h,&window)!=0)
        return ESP_ERR_INVALID_ARG;
    if(x==0 && w==320 && h==8 &&
       (y==0 || y==(int)WARSHIP_LCD_HEIGHT-8))
        ESP_LOGI(TAG,"PANEL_WINDOW x=%d y=%d"
                 " CASET_2A=%02x:%02x:%02x:%02x"
                 " RASET_2B=%02x:%02x:%02x:%02x RAMWR=0x2c bytes=%u",
                 x,y,(unsigned)window.caset[0],(unsigned)window.caset[1],
                 (unsigned)window.caset[2],(unsigned)window.caset[3],
                 (unsigned)window.raset[0],(unsigned)window.raset[1],
                 (unsigned)window.raset[2],(unsigned)window.raset[3],
                 (unsigned)(w*h*2));
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

/* Synchronous parameter writes. No color DMA is permitted until all
 * vendor commands and their delays have completed. Vendor errors are
 * terminal and force the backlight OFF, exactly as transport errors.
 */
static esp_err_t panel_send_commands(const char *stage,
                              const WarshipPanelCmd *commands, size_t count)
{
    if (commands == NULL || count == 0u || !panel_io || fault)
        return ESP_ERR_INVALID_STATE;
    for (size_t i=0u; i<count; ++i) {
        const WarshipPanelCmd *c=&commands[i];
        if(c->len>sizeof c->data) return sticky_error(ESP_ERR_INVALID_ARG);
        const esp_err_t err=esp_lcd_panel_io_tx_param(
            panel_io,c->cmd,c->len==0u?NULL:c->data,c->len);
        ESP_LOGI(TAG,"PANEL_PROFILE phase=%s step=%u cmd=0x%02x"
                 " length=%u delay_ms=%u err=%s",
                 stage,(unsigned)i,(unsigned)c->cmd,(unsigned)c->len,
                 (unsigned)c->delay_ms,esp_err_to_name(err));
        if(err!=ESP_OK) return sticky_error(err);
        if(c->delay_ms!=0u) vTaskDelay(pdMS_TO_TICKS(c->delay_ms));
    }
    return ESP_OK;
}

static void cleanup_before_dma(void)
{
    /* Never rely on GPIO_SET_LEVEL when LEDC owns GPIO21. */
    const esp_err_t light_err = warship_lcd_light_force_off();
    if (light_err != ESP_OK)
        ESP_LOGE(TAG, "CLEANUP_LIGHT_OFF_FAILED err=%s",
                 esp_err_to_name(light_err));
    if (panel) { (void)esp_lcd_panel_del(panel); panel = NULL; }
    if (panel_io) { (void)esp_lcd_panel_io_del(panel_io); panel_io = NULL; }
    if (owns_bus) { (void)spi_bus_free(LCD_SPI); owns_bus = false; }
    if (completed) { vSemaphoreDelete(completed); completed = NULL; }
    if (stripe) { heap_caps_free(stripe); stripe = NULL; }
}

esp_err_t warship_lcd_init(void)
{
    if (initialized || fault || panel || panel_io || owns_bus ||
        completed || stripe) return ESP_ERR_INVALID_STATE;

    /* Configures LEDC duty=0 and physically stops with idle_level=0
     * before any DMA / panel initialization. Fail closed if LEDC setup
     * is unavailable; never silently fall back to 100% GPIO HIGH. */
    esp_err_t err = warship_lcd_light_init();
    if (err != ESP_OK) {
        last_error = err;
        fault = true;
        (void)warship_lcd_light_force_off();
        ESP_LOGE(TAG,"BACKLIGHT_INIT_FATAL err=%s",
                 esp_err_to_name(err));
        return err;
    }

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
    /* ESP-IDF reset waits 20ms; add 130ms and send tested vendor init.
     * Do not call esp_lcd_panel_init(): it overwrites analog/gamma.
     */
    vTaskDelay(pdMS_TO_TICKS(130u));
    size_t n_init=0u;
    const WarshipPanelCmd *init=warship_panel_init_commands(&n_init);
    ESP_LOGW(TAG,"PANEL_PROFILE phase=INIT"
             " ref=TFT_eSPI_V2.5.43 source=ILI9341_2_DRIVER"
             " reset_wait_ms=150 profile=PRODUCTION");
    err=panel_send_commands("INIT",init,n_init);
    if(err!=ESP_OK) goto fail;

    initialized = true;
    ESP_LOGI(TAG, "INIT_PASS LCD=ILI9341 SPI2=13,14,15,2 BL=21 clock_hz=%u dma_bytes=%u",
             (unsigned)LCD_CLOCK, (unsigned)WARSHIP_LCD_DMA_BYTES);
    return ESP_OK;

fail:
    last_error = err;
    /* No DMA has been submitted in init(). Safe to release allocations. */
    cleanup_before_dma();
    fault = true; /* No retries after partially initialized LEDC/panel. */
    return err;
}

esp_err_t warship_lcd_prepare_console(uint16_t clear_rgb565)
{
    if(!initialized || fault || !panel_io || !panel || !stripe || !completed)
        return ESP_ERR_INVALID_STATE;
    if(prepared) {
        ESP_LOGI(TAG,"PANEL_PROFILE_REUSED madctl=0x28 no_clear=1");
        return ESP_OK;
    }
    /* Native landscape, BGR. Set MV=1 before any CASET/RASET/RAMWR;
     * esp_lcd mirror/swap cache must not overwrite the actual register.
     */
    const uint8_t madctl=warship_panel_landscape_madctl();
    esp_err_t err=esp_lcd_panel_io_tx_param(panel_io,0x36u,
                                            &madctl,sizeof madctl);
    ESP_LOGI(TAG,"PANEL_PROFILE phase=ROTATE_BEFORE_CLEAR"
             " cmd=0x36 madctl=0x%02x mv=1 mx=0 my=0 bgr=1 err=%s",
             (unsigned)madctl,esp_err_to_name(err));
    if(err!=ESP_OK) return sticky_error(err);

    for(unsigned row=0u;row<WARSHIP_LCD_HEIGHT;row+=WARSHIP_LCD_BAND_ROWS) {
        for(unsigned i=0u;i<WARSHIP_LCD_WIDTH*WARSHIP_LCD_BAND_ROWS;++i) {
            stripe[2u*i]=(uint8_t)(clear_rgb565>>8);
            stripe[2u*i+1u]=(uint8_t)clear_rgb565;
        }
        err=transfer(0,(int)row,(int)WARSHIP_LCD_WIDTH,
                     (int)WARSHIP_LCD_BAND_ROWS,
                     (int)WARSHIP_LCD_WIDTH,(int)WARSHIP_LCD_HEIGHT);
        if(err!=ESP_OK) return err;
    }
    ESP_LOGI(TAG,"PANEL_PROFILE phase=CLEAR_DONE madctl=0x28"
             " width=320 height=240 bands=30 dma=5120");
    /* Match tested sequence: gamma/inversion AFTER clear, BL still OFF. */
    size_t count=0u;
    const WarshipPanelCmd *commands=warship_panel_runtime_commands(&count);
    err=panel_send_commands("RUNTIME",commands,count);
    if(err!=ESP_OK) return err;
    prepared=true;
    last_error=ESP_OK;
    ESP_LOGW(TAG,"PANEL_PROFILE phase=READY"
             " madctl=0x28 gamma=DoomRPG_RE inversion=ON bgr=ON"
             " rgb565be_swap=NONE spi_hz=20000000 no_cached_rotation=1"
             " production=1");
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
    const esp_err_t err = warship_lcd_light_set(on);
    if (err != ESP_OK) return sticky_error(err);
    last_error = ESP_OK;
    return ESP_OK;
}

/* Explicit brightness adjustment for Pilotage experiments.
 * 0..100 valid. Bad user input does not make LCD transport sticky;
 * an actual LEDC hardware failure does.
 */
esp_err_t warship_lcd_backlight_set_percent(uint8_t percent)
{
    if (percent > 100u) return ESP_ERR_INVALID_ARG;
    if (!prepared || fault) return ESP_ERR_INVALID_STATE;
    const esp_err_t err = warship_lcd_light_set_percent(percent);
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
