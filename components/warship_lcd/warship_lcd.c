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
#ifdef WARSHIP_D14_COLOR_DIAGNOSTIC
#include "d14_color_diag.h"
#endif
#ifdef WARSHIP_D14_DOOM_REFERENCE
#include "d14_doom_ref.h"
#include "freertos/task.h"
#endif
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
    const esp_err_t light_err = warship_lcd_light_force_off();
    if (light_err != ESP_OK)
        ESP_LOGE(TAG, "BACKLIGHT_FORCE_OFF_FAILED err=%s",
                 esp_err_to_name(light_err));
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

#ifdef WARSHIP_D14_DOOM_REFERENCE
/* Synchronous parameter writes. No color DMA is permitted until all
 * vendor commands and their delays have completed. Vendor errors are
 * terminal and force the backlight OFF, exactly as transport errors.
 */
static esp_err_t d14_doom_send(const char *stage,
                              const D14DoomCmd *commands, size_t count)
{
    if (commands == NULL || count == 0u || !panel_io || fault)
        return ESP_ERR_INVALID_STATE;
    for (size_t i=0u; i<count; ++i) {
        const D14DoomCmd *c=&commands[i];
        if(c->len>sizeof c->data) return sticky_error(ESP_ERR_INVALID_ARG);
        const esp_err_t err=esp_lcd_panel_io_tx_param(
            panel_io,c->cmd,c->len==0u?NULL:c->data,c->len);
        ESP_LOGI(TAG,"D14_DOOM_PROFILE phase=%s step=%u cmd=0x%02x"
                 " length=%u delay_ms=%u err=%s",
                 stage,(unsigned)i,(unsigned)c->cmd,(unsigned)c->len,
                 (unsigned)c->delay_ms,esp_err_to_name(err));
        if(err!=ESP_OK) return sticky_error(err);
        if(c->delay_ms!=0u) vTaskDelay(pdMS_TO_TICKS(c->delay_ms));
    }
    return ESP_OK;
}
#endif

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
#ifdef WARSHIP_D14_DOOM_REFERENCE
    /* The native ESP-IDF panel object remains the DMA/CASET/RASET owner.
     * Bypass ONLY its default init list in this isolated probe. After
     * SWRESET it already waited 20ms; TFT_eSPI V2.5.43 waits 150ms.
     */
    vTaskDelay(pdMS_TO_TICKS(130u));
    size_t n_init=0u;
    const D14DoomCmd *init=d14_doom_init(&n_init);
    ESP_LOGW(TAG,"D14_DOOM_PROFILE phase=INIT"
             " ref=TFT_eSPI_V2.5.43 source=ILI9341_2_DRIVER"
             " reset_wait_ms=150 profile=TEST_ONLY");
    err=d14_doom_send("INIT",init,n_init);
    if(err!=ESP_OK) goto fail;
    /* DISPON was issued by the vendor list; do not call it twice. */
#else
    err = esp_lcd_panel_init(panel);
    if (err != ESP_OK) goto fail;
    err = esp_lcd_panel_disp_on_off(panel, true);
    if (err != ESP_OK) goto fail;
#endif

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

#ifdef WARSHIP_D14_DOOM_REFERENCE
esp_err_t warship_lcd_doom_reference_finish(void)
{
    if(!initialized || !prepared || fault || !panel_io || !panel ||
       !stripe || !completed || transfers != 30u)
        return ESP_ERR_INVALID_STATE;
    /* 30 case10 clear-band DMA transfers must have fully completed.
     * After this call no mirror()/swap_xy() may rewrite cached MADCTL.
     */
    size_t n=0u;
    const D14DoomCmd *cmds=d14_doom_runtime(&n);
    esp_err_t err=d14_doom_send("RUNTIME",cmds,n);
    if(err!=ESP_OK) return err;
    const uint8_t madctl=d14_doom_case10_madctl();
    err=esp_lcd_panel_io_tx_param(panel_io,0x36u,&madctl,1u);
    ESP_LOGI(TAG,"D14_DOOM_PROFILE phase=FINAL cmd=0x36"
             " madctl=0x%02x case10=1 bgr=1 mv=0 mx=0 my=1"
             " rgb565be_swap=NONE err=%s",(unsigned)madctl,
             esp_err_to_name(err));
    if(err!=ESP_OK) return sticky_error(err);
    ESP_LOGW(TAG,"D14_DOOM_PROFILE phase=DONE"
             " analogue=TFT_eSPI_V2.5.43 gamma=DoomRPG_RE"
             " inversion=ON bgr=ON case10=1"
             " product_firmware=UNCHANGED");
    return ESP_OK;
}
#endif

#ifdef WARSHIP_D14_COLOR_DIAGNOSTIC
/* TEST ONLY: main D14 diagnostic is the only caller.
 * Driver's case10 keeps MV=0, MX=0, MY=1. Explicit MADCTL
 * values 0x80 RGB / 0x88 BGR preserve that orientation.
 * Do NOT call mirror()/swap_xy() after the direct register write;
 * that would overwrite BGR using the cached driver state.
 */
static esp_err_t d14_vendor_param(char id, uint8_t cmd,
                                  const uint8_t *data,size_t length)
{
    const esp_err_t err=esp_lcd_panel_io_tx_param(panel_io,cmd,
                                                  data,length);
    ESP_LOGI(TAG,"D14_COLOR_CMD case=%c cmd=0x%02x bytes=%u err=%s",
             id,(unsigned)cmd,(unsigned)length,esp_err_to_name(err));
    if(err!=ESP_OK) return sticky_error(err);
    return ESP_OK;
}

esp_err_t warship_lcd_diag_apply_profile(unsigned index)
{
    static unsigned expected_index=0u;
    D14ColorProfile p;
    if(!initialized || !prepared || fault || !panel_io ||
       !panel || !stripe || !completed)
        return ESP_ERR_INVALID_STATE;
    if(index!=expected_index ||
       d14_color_profile(index,&p)!=0)
        return ESP_ERR_INVALID_ARG;

    /* The first three modes never write any gamma parameters:
     * A=RGB/OFF, B=RGB/ON, C=BGR/OFF (compared against A).
     * D restores RGB, applies the exact DoomRPG gamma bytes, then
     * enables inversion; E changes ONLY BGR relative to D.
     */
    ESP_LOGW(TAG,"D14_COLOR_PROFILE_BEGIN case=%c index=%u"
             " inverted=%u bgr=%u doom_gamma=%u"
             " madctl_case10=1 pwm_percent_experimental=40"
             " spi_hz=20000000 dma=5120",
             p.id,index,p.inverted,p.bgr,p.gamma);
    esp_err_t err;
    if(index==2u || index==3u || index==4u) {
        const uint8_t madctl=p.bgr ? 0x88u : 0x80u;
        ESP_LOGI(TAG,"D14_COLOR_MADCTL case=%c value=0x%02x"
                 " MY=1 MX=0 MV=0 BGR=%u",
                 p.id,(unsigned)madctl,(unsigned)p.bgr);
        err=d14_vendor_param(p.id,0x36u,&madctl,1u);
        if(err!=ESP_OK) return err;
    }
    if(index==3u) {
        const uint8_t zero=0x00u, one=0x01u;
        ESP_LOGI(TAG,"D14_COLOR_GAMMA_BYTES source=DoomRPG-RE"
                 " table=00:15:17:07:11:06:2B:56:3C:05:10:0F:3F:3F:0F");
        err=d14_vendor_param(p.id,0xf2u,&zero,1u);
        if(err!=ESP_OK) return err;
        err=d14_vendor_param(p.id,0x26u,&one,1u);
        if(err!=ESP_OK) return err;
        err=d14_vendor_param(p.id,0xe0u,d14_color_doom_gamma,15u);
        if(err!=ESP_OK) return err;
        err=d14_vendor_param(p.id,0xe1u,d14_color_doom_gamma,15u);
        if(err!=ESP_OK) return err;
    }
    err=esp_lcd_panel_invert_color(panel,p.inverted!=0u);
    ESP_LOGI(TAG,"D14_COLOR_INVERSION case=%c"
             " invert=%u command_expected=0x%02x err=%s",
             p.id,(unsigned)p.inverted,
             p.inverted ? 0x21u:0x20u,esp_err_to_name(err));
    if(err!=ESP_OK) return sticky_error(err);
    ESP_LOGW(TAG,"D14_COLOR_PROFILE_APPLIED case=%c"
             " rgb565be_payload_unchanged=1"
             " inverted=%u bgr=%u gamma=%u",p.id,
             (unsigned)p.inverted,(unsigned)p.bgr,(unsigned)p.gamma);
    ++expected_index;
    return ESP_OK;
}
#endif

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
