/* GUI-01 reception-only: reuses approved G01 layers, LCD DMA and SPI3 touch.
 * No D15 product navigation, no game, no NVS; only counters and logs.
 */
#include "d15a_g01.h"
#include "gui_button.h"
#include "t02_touch_map.h"
#include "warship_lcd.h"
#include "warship_touch.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <inttypes.h>

#define GUI01_DIM_LEVEL 170u /* 0..255; candidate 67%, pending CYD PO acceptance */
#define GUI01_POLL_MS 10u
#define GUI01_STABLE_SAMPLES 2u
static const char *const TAG="GUI01_TEST_ONLY";
static GuiButton buttons[2];
static GuiButtons group;
static uint32_t actions[2];
static uint32_t redraws, redraw_bytes, redraw_bands, touch_errors;

static esp_err_t redraw(GuiRect visual, unsigned layer, bool pressed,
                        uint32_t *transferred, uint32_t *bands, int64_t *us)
{
    GuiRect r;
    if (!gui_rect_clip(visual,D15A_W,D15A_H,&r)) return ESP_ERR_INVALID_ARG;
    const int64_t start=esp_timer_get_time();
    uint32_t sent=0, n=0;
    for (int32_t row=0;row<r.h;) {
        const int32_t remain=r.h-row;
        const uint16_t h=(uint16_t)(remain>8?8:remain);
        uint8_t *const dma=warship_lcd_band();
        if (!dma || d15a_compose_region((uint16_t)r.x,(uint16_t)(r.y+row),
                   (uint16_t)r.w,h,pressed?layer:D15A_IMAGE_COUNT,
                   GUI01_DIM_LEVEL,dma,WARSHIP_LCD_DMA_BYTES)!=0)
            return ESP_FAIL;
        /* warship_lcd_send waits for the on_color_trans_done semaphore.
         * No writer reuses DMA memory before the completed transaction. */
        const esp_err_t err=warship_lcd_send((int)r.x,(int)(r.y+row),
                                             (int)r.w,(int)h);
        if (err!=ESP_OK) return err;
        sent+=(uint32_t)r.w*h*2u;++n;row+=h;
    }
    *transferred=sent;*bands=n;*us=esp_timer_get_time()-start;
    return ESP_OK;
}
static bool handle_input(GuiInput input, int32_t x, int32_t y, int64_t sample_us)
{
    const GuiButtonEvent e=gui_buttons_handle(&group,input,x,y);
    if (e.redraw) {
        uint32_t bytes=0, bands=0;int64_t duration=0;
        const bool pressed=buttons[e.index].state==GUI_BUTTON_PRESSED;
        const unsigned layer=e.index==0?D15A_JOUER:D15A_PALMARES;
        const esp_err_t err=redraw(buttons[e.index].visual,layer,pressed,
                                    &bytes,&bands,&duration);
        if (err!=ESP_OK) {
            ESP_LOGE(TAG,"GUI01_RENDER_FAIL action=%u error=%s",
                     (unsigned)buttons[e.index].action_id,esp_err_to_name(err));
            (void)warship_lcd_backlight(false);
            return false;
        }
        ++redraws;redraw_bytes+=bytes;redraw_bands+=bands;
        ESP_LOGI(TAG,"GUI01_REDRAW action=%u state=%s bytes=%" PRIu32
                 " bands=%" PRIu32 " draw_us=%" PRId64 " sample_to_feedback_us=%" PRId64,
                 (unsigned)buttons[e.index].action_id,pressed?"PRESSED":"NORMAL",
                 bytes,bands,duration,esp_timer_get_time()-sample_us);
    }
    if (e.cancelled) ESP_LOGI(TAG,"GUI01_CANCEL reason=%d",(int)input);
    if (e.activated) {
        const unsigned idx=e.action_id==1?0:1;
        ++actions[idx];
        ESP_LOGI(TAG,"GUI01_ACTIVATE id=%u count=%" PRIu32,
                 (unsigned)e.action_id,actions[idx]);
    }
    return true;
}
static bool sample_point(int32_t *x, int32_t *y)
{
    WarshipTouchSample raw;
    uint16_t px=0,py=0;
    const esp_err_t err=warship_touch_sample(&raw);
    if (err!=ESP_OK || raw.x<=1u || raw.y<=1u ||
        raw.x>=4094u || raw.y>=4094u ||
        t02_map_d09_point(raw.x,raw.y,&px,&py)!=0) {
        ++touch_errors;
        ESP_LOGW(TAG,"GUI01_TOUCH_INVALID err=%s count=%" PRIu32,
                 esp_err_to_name(err),touch_errors);
        return false;
    }
    *x=(int32_t)px;*y=(int32_t)py;
    return true;
}
void app_main(void)
{
    ESP_LOGW(TAG,"WARSHIP_GUI01_TEST_ONLY=ON NO_GAME=1 NO_NVS=1"
             " NO_NAVIGATION=1 SPI3_OWNER=GUI01 BUTTON_DIM=%u/255"
             " POLL_MS=%u DEBOUNCE_SAMPLES=%u",GUI01_DIM_LEVEL,
             GUI01_POLL_MS,GUI01_STABLE_SAMPLES);
    ESP_LOGI(TAG,"GUI01_APPROVED_ASSETS=wallpaper.accueil.retenu.cyd,"
             "logo.titre.cyd,boutons.jouer.cyd,boutons.palmares.cyd");
    if (!gui_button_init(&buttons[0],(GuiRect){14,198,140,32},
                          (GuiRect){8,190,152,46},1,true) ||
        !gui_button_init(&buttons[1],(GuiRect){166,199,140,30},
                          (GuiRect){160,190,152,46},2,true) ||
        !gui_buttons_init(&group,buttons,2)) {
        ESP_LOGE(TAG,"GUI01_BUTTON_INIT_FAIL");return;
    }
    esp_err_t err=warship_lcd_init();
    if (err!=ESP_OK) {ESP_LOGE(TAG,"LCD_INIT_FAIL %s",esp_err_to_name(err));return;}
    err=warship_lcd_prepare_console(0);
    if (err!=ESP_OK) {ESP_LOGE(TAG,"LCD_PROFILE_FAIL %s",esp_err_to_name(err));return;}
    for (uint16_t row=0;row<D15A_H;row+=D15A_BAND_ROWS) {
        uint8_t *dma=warship_lcd_band();
        if (!dma || d15a_compose_band(row,D15A_BAND_ROWS,dma,WARSHIP_LCD_DMA_BYTES)!=0 ||
            (err=warship_lcd_send(0,row,D15A_W,D15A_BAND_ROWS))!=ESP_OK) {
            ESP_LOGE(TAG,"GUI01_INITIAL_DRAW_FAIL row=%u",(unsigned)row);
            (void)warship_lcd_backlight(false);return;
        }
    }
    err=warship_lcd_backlight(true);
    if (err!=ESP_OK) {ESP_LOGE(TAG,"BACKLIGHT_FAIL %s",esp_err_to_name(err));return;}
    err=warship_touch_init(); /* sole SPI3 owner in this firmware */
    if (err!=ESP_OK) {
        ESP_LOGE(TAG,"TOUCH_INIT_FAIL %s",esp_err_to_name(err));
        (void)warship_lcd_backlight(false);return;
    }
    ESP_LOGI(TAG,"GUI01_READY hitbox_left=[8,160)x[190,236)"
             " hitbox_right=[160,312)x[190,236) status=RECEPTION_ONLY");
    bool stable=false,has_last=false;
    unsigned lows=0,highs=0;
    int32_t last_x=0,last_y=0;
    for (;;) {
        const bool irq=warship_touch_pressed();
        if (irq) {
            highs=0;
            if (lows<GUI01_STABLE_SAMPLES) ++lows;
            if (stable || lows==GUI01_STABLE_SAMPLES) {
                const int64_t t=esp_timer_get_time();
                int32_t x=0,y=0;
                const bool valid=sample_point(&x,&y);
                if (!stable) {
                    stable=true;has_last=valid;
                    if (!handle_input(GUI_INPUT_DOWN,valid?x:-1,valid?y:-1,t))break;
                } else if (!valid) {
                    has_last=false;
                    if (!handle_input(GUI_INPUT_INVALID,0,0,t))break;
                } else {
                    has_last=true;
                    if (!handle_input(GUI_INPUT_MOVE,x,y,t))break;
                }
                if (valid) {last_x=x;last_y=y;}
            }
        } else {
            lows=0;
            if (stable) {
                if (highs<GUI01_STABLE_SAMPLES)++highs;
                if (highs==GUI01_STABLE_SAMPLES) {
                    stable=false;highs=0;
                    const int64_t t=esp_timer_get_time();
                    if (!handle_input(has_last?GUI_INPUT_UP:GUI_INPUT_INVALID,
                                      last_x,last_y,t))break;
                    if (!has_last && !handle_input(GUI_INPUT_UP,0,0,t))break;
                    has_last=false;
                    WarshipLcdInfo info={0};
                    if (warship_lcd_info(&info))
                        ESP_LOGI(TAG,"GUI01_COUNTERS redraws=%" PRIu32
                                 " bytes=%" PRIu32 " bands=%" PRIu32
                                 " touch_errors=%" PRIu32 " lcd_fault=%d"
                                 " timeouts=%" PRIu32,
                                 redraws,redraw_bytes,redraw_bands,touch_errors,
                                 (int)info.dma_fault,info.timeouts);
                    ESP_LOGI(TAG,"GUI01_MEM heap8=%u largest8=%u stack_free_bytes=%u",
                             (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
                             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
                             (unsigned)uxTaskGetStackHighWaterMark(NULL));
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(GUI01_POLL_MS));
    }
    ESP_LOGE(TAG,"GUI01_FATAL no_retry=1");
    for (;;) vTaskDelay(pdMS_TO_TICKS(1000));
}
