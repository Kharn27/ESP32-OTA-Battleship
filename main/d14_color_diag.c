/* D14 COLOR TEST ONLY: a single diagnostic CYD, no NVS/touch/game loop.
 * Sequential A/B/C/D/E; every case draws 320x240 directly from constants
 * then draws the SAME immutable G03 through the RLE+alpha compositor.
 * SPI2 20MHz, MADCTL case10, PWM 40%, one 5120B borrowed DMA buffer.
 */
#include "d14_color_diag.h"
#include "d14_g03.h"
#include "warship_lcd.h"

#include <inttypes.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define D14_COLOR_HOLD_MS 3500u

static const char *const TAG = "D14_COLOR";

/* Hand-constructed canonical G03 layout; public frame only, never CPU.
 * Same five ships 2/3/3/4/5 as G03 preview. No game initialization.
 */
static void make_public_frame(WarshipDeployFrame *f)
{
    for(unsigned i=0u;i<100u;++i) f->cells[i]=0u;
    f->mode=1u;
    for(unsigned col=6u;col<8u;++col) f->cells[7u*10u+col]=1u;
    for(unsigned col=2u;col<5u;++col) f->cells[5u*10u+col]=2u;
    for(unsigned row=4u;row<7u;++row) f->cells[row*10u+5u]=3u;
    for(unsigned row=3u;row<7u;++row) f->cells[row*10u+8u]=4u;
    for(unsigned col=1u;col<6u;++col) f->cells[1u*10u+col]=5u;
}

static void diagnostic_metrics(char id,const char *phase)
{
    WarshipLcdInfo lcd={0};
    (void)warship_lcd_info(&lcd);
    ESP_LOGI(TAG,"D14_COLOR_MEM case=%c phase=%s"
             " heap8=%u largest8=%u stack_margin_bytes=%u"
             " lcd_bands=%u lcd_fault=%u timeouts=%u last_error=%s",
             id,phase,
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
             (unsigned)uxTaskGetStackHighWaterMark(NULL),
             (unsigned)lcd.transfers,(unsigned)lcd.dma_fault,
             (unsigned)lcd.timeouts,esp_err_to_name(lcd.last_error));
}

static esp_err_t diagnostic_draw(const WarshipDeployFrame *frame,
                                  bool g03,char id)
{
    uint8_t *band=warship_lcd_band();
    if(!band) return ESP_ERR_INVALID_STATE;
    const int64_t start=esp_timer_get_time();
    int64_t compose_us=0,lcd_us=0;
    unsigned bands=0u;
    for(unsigned top=0u;top<240u;top+=D14_COLOR_BAND_ROWS) {
        const int64_t start_compose=esp_timer_get_time();
        int status;
        if(g03)
            status=d14_g03_compose(frame,0u,(uint16_t)top,320u,8u,
                                   band,WARSHIP_LCD_DMA_BYTES);
        else
            status=d14_color_direct_band((uint16_t)top,8u,band,
                                          WARSHIP_LCD_DMA_BYTES);
        compose_us+=esp_timer_get_time()-start_compose;
        if(status!=0) return ESP_ERR_INVALID_ARG;
        const int64_t start_lcd=esp_timer_get_time();
        const esp_err_t err=warship_lcd_send(0,(int)top,320,8);
        lcd_us+=esp_timer_get_time()-start_lcd;
        if(err!=ESP_OK) return err;
        ++bands;
    }
    ESP_LOGI(TAG,"D14_COLOR_DRAW case=%c page=%s bands=%u"
             " compose_us=%" PRId64 " lcd_us=%" PRId64
             " total_us=%" PRId64 " dma_bytes=%u",
             id,g03?"G03_RLE_ALPHA":"DIRECT_RGB565",
             bands,compose_us,lcd_us,esp_timer_get_time()-start,
             (unsigned)WARSHIP_LCD_DMA_BYTES);
    return ESP_OK;
}

void d14_color_diag_run(void)
{
    ESP_LOGW(TAG,"D14_COLOR_DIAGNOSTIC=ON TEST_ONLY no_nvs=1"
             " no_touch=1 no_gameplay=1 no_flash_writes=1"
             " NO_PRODUCT_COLOR_DECISION=1");
    ESP_LOGI(TAG,"D14_COLOR_REFS"
             " Loco_BASIC=esp_lcd_RGB_BIG_SPI2_20MHz_orientation_9_or_10"
             " DoomRPG=ILI9341_2_BGR_INVERT_ON_F2_26_E0_E1");
    ESP_LOGI(TAG,"D14_COLOR_PIXELS RGB565_BE"
             " BLACK=0000(00:00) WHITE=FFFF(FF:FF)"
             " RED=F800(F8:00) GREEN=07E0(07:E0)"
             " BLUE=001F(00:1F) GRAY=8410(84:10)"
             " YELLOW=FFE0(FF:E0) RAMP_0_to_FFFF=1");

    esp_err_t err=warship_lcd_init();
    if(err==ESP_OK) err=warship_lcd_prepare_console(0x000cu);
    if(err!=ESP_OK) goto failed;
    diagnostic_metrics('-', "lcd_profile_prepared");

    WarshipDeployFrame f;
    make_public_frame(&f);
    for(unsigned index=0;index<D14_COLOR_PROFILES;++index) {
        D14ColorProfile profile;
        if(d14_color_profile(index,&profile)!=0) {
            err=ESP_ERR_INVALID_ARG;
            goto failed;
        }
        ESP_LOGW(TAG,"D14_COLOR_STEP case=%c index=%u"
                 " inv=%u bgr=%u gamma=%u"
                 " next_page=DIRECT_RGB565 then G03_RLE_ALPHA"
                 " hold_ms_each=%u",
                 profile.id,index,(unsigned)profile.inverted,
                 (unsigned)profile.bgr,(unsigned)profile.gamma,
                 (unsigned)D14_COLOR_HOLD_MS);
        /* This TEST ONLY function uses the genuine esp_lcd panel and IO.
         * A/B/C do not modify gamma. D writes exact DoomRPG gamma;
         * E flips only MADCTL.BGR after D. Each step is logged.
         */
        err=warship_lcd_diag_apply_profile(index);
        if(err!=ESP_OK) goto failed;
        err=diagnostic_draw(&f,false,profile.id);
        if(err!=ESP_OK) goto failed;
        if(index==0u) {
            err=warship_lcd_backlight(true);
            if(err!=ESP_OK) goto failed;
        }
        diagnostic_metrics(profile.id,"DIRECT_RGB565");
        vTaskDelay(pdMS_TO_TICKS(D14_COLOR_HOLD_MS));
        err=diagnostic_draw(&f,true,profile.id);
        if(err!=ESP_OK) goto failed;
        diagnostic_metrics(profile.id,"G03_RLE_ALPHA");
        vTaskDelay(pdMS_TO_TICKS(D14_COLOR_HOLD_MS));
    }
    ESP_LOGW(TAG,"D14_COLOR_DONE pages=10 profiles=A_B_C_D_E"
             " final_case=E stop_requires_power_cycle=1"
             " G03_source_assets_unchanged=1");
    diagnostic_metrics('E',"completed");
    for(;;) vTaskDelay(pdMS_TO_TICKS(1000));

failed:
    ESP_LOGE(TAG,"D14_COLOR_ABORT err=%s LCD_STOP_NO_RETRY=1",
             esp_err_to_name(err));
    const esp_err_t off=warship_lcd_backlight(false);
    ESP_LOGI(TAG,"D14_COLOR_BL_FORCE_OFF_RESULT err=%s",
             esp_err_to_name(off));
    diagnostic_metrics('!',"fault");
}
