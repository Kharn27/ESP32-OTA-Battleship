/* T02-B: guided raw calibration on ONE CYD, not D09, not game UI.
 * Does not modify or apply D09 hitboxes, no NVS and no auto-calibration.
 */
#include "t02_cal_probe.h"
#include "t02_cal.h"
#include "warship_lcd.h"
#include "warship_touch.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *const TAG="T02_CAL";
static T02Cal cal;
static uint32_t spi_errors, rejects, accepted;

static void memory_log(const char *phase)
{
    WarshipLcdInfo info={0};
    (void)warship_lcd_info(&info);
    ESP_LOGI(TAG,"MEM phase=%s heap8=%u largest8=%u min8=%u dma_free=%u dma_largest=%u stack_margin_bytes=%u lcd_dma_bytes=%u lcd_transfers=%u dma_timeouts=%u dma_fault=%u cal_bytes=%u",
             phase,(unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA),
             (unsigned)uxTaskGetStackHighWaterMark(NULL),
             (unsigned)WARSHIP_LCD_DMA_BYTES,(unsigned)info.transfers,
             (unsigned)info.timeouts,(unsigned)info.dma_fault,
             (unsigned)sizeof cal);
}
static esp_err_t draw(void)
{
    uint8_t *const band=warship_lcd_band();
    if (band==NULL) return ESP_ERR_INVALID_STATE;
    const int64_t started=esp_timer_get_time();
    for (unsigned top=0u;top<240u;top+=WARSHIP_LCD_BAND_ROWS) {
        for (unsigned row=0u;row<WARSHIP_LCD_BAND_ROWS;++row) {
            for (unsigned x=0u;x<320u;++x) {
                const uint16_t rgb=t02_cal_pixel(&cal,(uint16_t)x,
                                                   (uint16_t)(top+row));
                const size_t offset=2u*(row*320u+x);
                band[offset]=(uint8_t)(rgb>>8);
                band[offset+1u]=(uint8_t)rgb;
            }
        }
        const esp_err_t err=warship_lcd_send(0,(int)top,320,
                                             (int)WARSHIP_LCD_BAND_ROWS);
        if (err!=ESP_OK) {
            ESP_LOGE(TAG,"LCD_REFRESH_STOP y=%u err=%s",top,
                     esp_err_to_name(err));
            return err;
        }
    }
    ESP_LOGI(TAG,"DRAW point=%u phase=%u count=%u us=%" PRId64 " bands=30 dma=5120",
             (unsigned)(cal.point+1u),(unsigned)cal.phase,
             (unsigned)cal.count,esp_timer_get_time()-started);
    return ESP_OK;
}
static void completion_report(void)
{
    for (uint8_t p=0u;p<T02_CAL_POINTS;++p) {
        T02CalStats s;
        if (t02_cal_stats(&cal,p,&s)!=0) continue;
        uint16_t tx,ty;
        t02_cal_target(p,&tx,&ty);
        ESP_LOGI(TAG,"CAL_POINT point=%u target=%u,%u median=%u,%u spread=%u,%u",
                 (unsigned)(p+1u),(unsigned)tx,(unsigned)ty,
                 (unsigned)s.median.x,(unsigned)s.median.y,
                 (unsigned)s.spread_x,(unsigned)s.spread_y);
        for (uint8_t i=0u;i<3u;++i) {
            const T02CalRaw v=cal.samples[p][i];
            ESP_LOGI(TAG,"CAL_RAW point=%u press=%u x=%u y=%u",
                     (unsigned)(p+1u),(unsigned)(i+1u),
                     (unsigned)v.x,(unsigned)v.y);
        }
    }
    T02CalMapping map;
    if (t02_cal_solve(&cal,&map)==0) {
        ESP_LOGW(TAG,"CAL_PROPOSAL NOT_APPLIED swap=%u x0=%u x1=%u y0=%u y1=%u max_err_px=%u sum_sq_px=%u",
                 (unsigned)map.swap,(unsigned)map.x0,(unsigned)map.x1,
                 (unsigned)map.y0,(unsigned)map.y1,
                 (unsigned)map.max_error_px,(unsigned)map.sum_square_error);
    } else {
        ESP_LOGW(TAG,"CAL_PROPOSAL UNDETERMINED (degenerate/ambiguous axes); NO D09 CHANGE");
    }
    memory_log("cal_done");
}
void t02_cal_probe_run(void)
{
    ESP_LOGW(TAG,"BOOT_MODE=T02_CAL GUIDED 5 POINTS x 3 PRESSES; NO D09 MAPPING CHANGE");
    ESP_LOGI(TAG,"CONTROLS short press/release >=40ms then release >=80ms; hold >=2200ms RETRY point; REVIEW tap NEXT; DONE hold RESTART");
    t02_cal_init(&cal);
    memory_log("startup");
    esp_err_t err=warship_lcd_init();
    if (err!=ESP_OK) {
        ESP_LOGE(TAG,"LCD_INIT_FAIL err=%s",esp_err_to_name(err));
        return;
    }
    err=warship_lcd_prepare_console(0x000cu);
    if (err!=ESP_OK) {
        ESP_LOGE(TAG,"LCD_PROFILE_FAIL err=%s",esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG,"LCD_PROFILE_PASS case=10 native_wide=1 dma=5120");
    memory_log("lcd_ready");
    if (draw()!=ESP_OK) return;
    err=warship_lcd_backlight(true);
    if (err!=ESP_OK) {
        ESP_LOGE(TAG,"BACKLIGHT_FAIL err=%s",esp_err_to_name(err));
        return;
    }
    err=warship_touch_init();
    if (err!=ESP_OK) {
        ESP_LOGE(TAG,"TOUCH_INIT_FAIL err=%s",esp_err_to_name(err));
        return;
    }
    memory_log("touch_ready");
    bool prev_down=false;
    uint64_t last_mem=(uint64_t)esp_timer_get_time()/1000u;
    WarshipTouchSample latest={0};
    for (;;) {
        const uint64_t ms=(uint64_t)esp_timer_get_time()/1000u;
        const bool down=warship_touch_pressed();
        uint8_t valid=down?0u:1u;
        if (down) {
            WarshipTouchSample raw;
            err=warship_touch_sample(&raw);
            if (err!=ESP_OK) {
                ++spi_errors;
                ESP_LOGE(TAG,"TOUCH_SPI_ERROR count=%u err=%s",
                         (unsigned)spi_errors,esp_err_to_name(err));
            } else {
                latest=raw;
                valid=(uint8_t)(raw.x>0u && raw.x<4095u &&
                                raw.y>0u && raw.y<4095u);
                if (!valid)
                    ESP_LOGW(TAG,"CAL_INVALID_ADC raw=%u,%u",
                             (unsigned)raw.x,(unsigned)raw.y);
            }
        }
        if (down && !prev_down)
            ESP_LOGI(TAG,"CAL_DOWN point=%u raw=%u,%u valid=%u",
                     (unsigned)(cal.point+1u),(unsigned)latest.x,
                     (unsigned)latest.y,(unsigned)valid);
        if (!down && prev_down) ESP_LOGI(TAG,"CAL_UP point=%u",(unsigned)(cal.point+1u));
        prev_down=down;
        const T02CalEvent event=t02_cal_step(&cal,(uint8_t)down,valid,
                                             latest.x,latest.y,ms);
        if (event!=T02_CAL_NONE) {
            ESP_LOGI(TAG,"CAL_EVENT id=%u point=%u count=%u phase=%u",
                     (unsigned)event,(unsigned)(cal.point+1u),
                     (unsigned)cal.count,(unsigned)cal.phase);
            if (event==T02_CAL_REJECT) ++rejects;
            if (event==T02_CAL_ACCEPT) {
                ++accepted;
                const T02CalRaw sample=cal.samples[cal.point][cal.count-1u];
                ESP_LOGI(TAG,"CAL_ACCEPT point=%u press=%u raw=%u,%u adc_x=%u,%u,%u adc_y=%u,%u,%u",
                         (unsigned)(cal.point+1u),(unsigned)cal.count,
                         (unsigned)sample.x,(unsigned)sample.y,
                         (unsigned)latest.xs[0],(unsigned)latest.xs[1],
                         (unsigned)latest.xs[2],(unsigned)latest.ys[0],
                         (unsigned)latest.ys[1],(unsigned)latest.ys[2]);
                if (cal.phase==T02_CAL_REVIEW) {
                    T02CalStats s;
                    if (t02_cal_stats(&cal,cal.point,&s)==0)
                        ESP_LOGI(TAG,"CAL_SPREAD point=%u raw_dx=%u raw_dy=%u median=%u,%u",
                                 (unsigned)(cal.point+1u),
                                 (unsigned)s.spread_x,(unsigned)s.spread_y,
                                 (unsigned)s.median.x,(unsigned)s.median.y);
                }
            }
            if (event==T02_CAL_COMPLETE) completion_report();
            /* Only redraw on state changes; reject leaves visual state alone. */
            if (event!=T02_CAL_REJECT && draw()!=ESP_OK) {
                memory_log("draw_fault");
                return;
            }
        }
        if (ms-last_mem>=10000u) {
            memory_log("running");
            ESP_LOGI(TAG,"COUNTERS accepted=%u rejects=%u spi_errors=%u",
                     (unsigned)accepted,(unsigned)rejects,(unsigned)spi_errors);
            last_mem=ms;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
