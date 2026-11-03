/* D09+D10 technical presentation, D11 post-ACK NVS event handoff. A
 * single task owns both SPI buses, lifecycle, DMA and NVS writes. No radio. */
#include "d09_probe.h"
#include "d11_runtime.h"
#include "warship_deploy_ui.h"
#include "warship_combat_ui.h"
#include "warship_lcd.h"
#include "warship_touch.h"
#include "t02_touch_map.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

/* Explicit minimum required by D09 P1; sdkconfig.defaults is only used
 * when no higher-priority local sdkconfig override exists. Fail a stale
 * local build rather than quietly ship a 3584-byte main-task stack again.
 */
#if CONFIG_ESP_MAIN_TASK_STACK_SIZE < 4096
#error "D09 needs >=4096 B app_main stack; regenerate sdkconfig (idf.py set-target esp32) or configure main task stack >=4096"
#endif

static const char *const TAG="D09";
static WarshipLifecycle game;
static WarshipLifecyclePublic public_view;
static WarshipDeployFrame frame;
static WarshipDeployInput input;
static WarshipCombatUi combat;
static WarshipCombatInput combat_input;
static WarshipCombatFrame combat_frame;

static int rng_u32(void *ctx,uint32_t *out)
{
    (void)ctx;
    if(out==NULL)return -1;
    *out=esp_random();
    return 0;
}
static void memory_log(const char *phase)
{
    WarshipLcdInfo lcd={0};
    (void)warship_lcd_info(&lcd);
    ESP_LOGI(TAG,"MEM phase=%s heap8=%u largest8=%u minimum8=%u dma_free=%u dma_largest=%u stack_margin_bytes=%u lcd_dma=%u bands=%u timeouts=%u lcd_fault=%u sizeof_game=%u sizeof_public=%u sizeof_frame=%u",
             phase,
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL|MALLOC_CAP_DMA),
             (unsigned)uxTaskGetStackHighWaterMark(NULL),
             (unsigned)WARSHIP_LCD_DMA_BYTES,
             (unsigned)lcd.transfers,(unsigned)lcd.timeouts,
             (unsigned)lcd.dma_fault,
             (unsigned)sizeof game,(unsigned)sizeof public_view,
             (unsigned)sizeof frame);
}
static esp_err_t draw_region(int x,int y,int w,int h)
{
    if(x<0 || y<0 || w<=0 || h<=0 || x >= (int)WARSHIP_LCD_WIDTH ||
       y >= (int)WARSHIP_LCD_HEIGHT || w>(int)WARSHIP_LCD_WIDTH-x ||
       h>(int)WARSHIP_LCD_HEIGHT-y ||
       (size_t)w*WARSHIP_LCD_BAND_ROWS*2u>WARSHIP_LCD_DMA_BYTES)
        return ESP_ERR_INVALID_ARG;
    uint8_t *const band=warship_lcd_band();
    if(band==NULL)return ESP_ERR_INVALID_STATE;
    const int64_t begin=esp_timer_get_time();
    unsigned sent=0u;
    for(int top=y; top<y+h; top+=(int)WARSHIP_LCD_BAND_ROWS) {
        const int rows=(y+h-top)<(int)WARSHIP_LCD_BAND_ROWS ?
                       (y+h-top):(int)WARSHIP_LCD_BAND_ROWS;
        for(int r=0;r<rows;++r) {
            for(int col=0;col<w;++col) {
                const uint16_t color=(game.phase==WARSHIP_LIFECYCLE_COMBAT ||
                                      game.phase==WARSHIP_LIFECYCLE_RESULT) ?
                    warship_combat_pixel(&combat_frame,(uint16_t)(x+col),
                                          (uint16_t)(top+r)) :
                    warship_deploy_pixel(&frame,&input.layout,
                                         (uint16_t)(x+col),(uint16_t)(top+r));
                const size_t at=2u*((size_t)r*(size_t)w+(size_t)col);
                band[at]=(uint8_t)(color>>8);
                band[at+1u]=(uint8_t)color;
            }
        }
        const esp_err_t err=warship_lcd_send(x,top,w,rows);
        if(err!=ESP_OK) {
            ESP_LOGE(TAG,"DRAW_FAIL region=%d,%d,%d,%d band_y=%d err=%s",
                     x,y,w,h,top,esp_err_to_name(err));
            return err; /* No buffer overwrite after unknown DMA fault. */
        }
        ++sent;
    }
    ESP_LOGI(TAG,"DRAW x=%d y=%d w=%d h=%d bands=%u us=%" PRId64,
             x,y,w,h,sent,esp_timer_get_time()-begin);
    return ESP_OK;
}
static esp_err_t draw_combat(uint8_t include_pending)
{
    if (warship_lifecycle_observe(&game,&public_view)!=WARSHIP_LIFECYCLE_OK ||
        warship_combat_make_frame(&public_view,&combat,include_pending,
                                  &combat_frame)!=WARSHIP_COMBAT_OK) {
        ESP_LOGE(TAG,"D10_PUBLIC_FRAME_FAIL pending=%u",
                 (unsigned)include_pending);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG,"COMBAT_FRAME view=%u busy=%u reveal=%u round=%u terminal=%u alert=%u",
             (unsigned)combat_frame.view,(unsigned)combat_frame.busy,
             (unsigned)include_pending,(unsigned)combat_frame.rounds,
             (unsigned)combat_frame.terminal,(unsigned)combat_frame.alert);
    return draw_region(0,0,320,240);
}
/* Refresh only the previous/new 19px cell and FEU button for a
 * selection toggle/replacement. Full frame reserved for view/FEU/reveal.
 */
static esp_err_t draw_combat_selection(uint8_t had, uint8_t old_x,
                                      uint8_t old_y)
{
    if(warship_lifecycle_observe(&game,&public_view)!=WARSHIP_LIFECYCLE_OK ||
       warship_combat_make_frame(&public_view,&combat,0u,
                                 &combat_frame)!=WARSHIP_COMBAT_OK)
        return ESP_FAIL;
    if(had) {
        const esp_err_t e=draw_region(22+(int)old_x*19,24+(int)old_y*19,19,19);
        if(e!=ESP_OK) return e;
    }
    if(combat_frame.has_selection &&
       (!had || old_x!=combat_frame.target_x ||
        old_y!=combat_frame.target_y)) {
        const esp_err_t e=draw_region(
            22+(int)combat_frame.target_x*19,
            24+(int)combat_frame.target_y*19,19,19);
        if(e!=ESP_OK) return e;
    }
    return draw_region(225,32,91,38);
}
static esp_err_t draw_for_action(WarshipDeployAction action)
{
    if (game.phase==WARSHIP_LIFECYCLE_COMBAT)
        return draw_combat(0u);
    if(warship_lifecycle_observe(&game,&public_view)!=WARSHIP_LIFECYCLE_OK) {
        ESP_LOGE(TAG,"D07_OBSERVE_FAIL");
        return ESP_FAIL;
    }
    const WarshipDeployStatus st=warship_deploy_make_frame(&public_view,&frame);
    if(st!=WARSHIP_DEPLOY_OK) {
        ESP_LOGE(TAG,"FRAME_INVALID status=%u",(unsigned)st);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG,"FRAME phase=%u mode=%u public_human=%u public_battle=%u",
             (unsigned)public_view.phase,(unsigned)frame.mode,
             (unsigned)public_view.has_human_fleet,
             (unsigned)public_view.has_battle);
    if(action==WARSHIP_DEPLOY_AUTO)
        return draw_region(0,0,220,240);
    if(action==WARSHIP_DEPLOY_READY)
        return draw_region(220,0,100,240);
    return draw_region(0,0,320,240);
}
void d09_probe_run(void)
{
    ESP_LOGW(TAG,"BOOT probe=D09 technical only; PO calibration issue=21 swap=1 screen_x_raw_y=215..3799 screen_y_raw_x=302..3865 size=320x240; NOT G03");
    ESP_LOGI(TAG,"STACK_CONFIG main_task_bytes=%u sizeof_solo=%u sizeof_battle=%u; MAIN task watermark is MIN FREE BYTES, not current free stack",
             (unsigned)CONFIG_ESP_MAIN_TASK_STACK_SIZE,
             (unsigned)sizeof(WarshipSoloSession),
             (unsigned)sizeof(WarshipBattle));
    memory_log("startup_after_nvs_read");
    WarshipDeployLayout layout=warship_deploy_default_layout();
    if(warship_deploy_input_init(&input,&layout)!=WARSHIP_DEPLOY_OK ||
       warship_lifecycle_init(&game)!=WARSHIP_LIFECYCLE_OK) {
        ESP_LOGE(TAG,"D09_CONTROL_INIT_FAIL");
        return;
    }
    /* Initial entry is already deployed (LOT1-PL-001); START button
     * is a TECHNICAL restart after RETOUR, not the product home UI. */
    if(warship_deploy_apply(&game,WARSHIP_DEPLOY_START,
                            rng_u32,NULL,rng_u32,NULL)!=WARSHIP_DEPLOY_OK) {
        ESP_LOGE(TAG,"D09_FLEET_GENERATION_FAIL");
        return;
    }
    esp_err_t err=warship_lcd_init();
    if(err!=ESP_OK) {
        ESP_LOGE(TAG,"LCD_INIT_FAIL err=%s",esp_err_to_name(err));
        return;
    }
    err=warship_lcd_prepare_console(0x000cu);
    if(err!=ESP_OK) {
        ESP_LOGE(TAG,"LCD_PROFILE_FAIL err=%s",esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG,"LCD_PROFILE_PASS case=10 native_wide=1 mv=0 mx=0 my=1 dma=5120");
    memory_log("lcd_ready");
    err=draw_for_action(WARSHIP_DEPLOY_START);
    if(err!=ESP_OK) {
        ESP_LOGE(TAG,"D09_DRAW_STOP err=%s",esp_err_to_name(err));
        memory_log("lcd_fault");
        return;
    }
    err=warship_lcd_backlight(true);
    if(err!=ESP_OK) {
        ESP_LOGE(TAG,"LCD_BACKLIGHT_FAIL err=%s",esp_err_to_name(err));
        return;
    }
    if(warship_touch_init()!=ESP_OK) {
        ESP_LOGE(TAG,"TOUCH_INIT_FAIL_NO_ACTIONS");
        memory_log("touch_init_fail");
        return;
    }
    memory_log("ready");
    uint64_t last_mem=(uint64_t)(esp_timer_get_time()/1000);
    uint32_t sample_errors=0u, action_count=0u;
    bool prev_down=false;
    for(;;) {
        const uint64_t ms=(uint64_t)(esp_timer_get_time()/1000);
        const bool down=warship_touch_pressed();
        uint8_t valid=1u;
        uint16_t x=0u,y=0u;
        WarshipTouchSample raw={0};
        if(down) {
            const esp_err_t spi=warship_touch_sample(&raw);
            if(spi!=ESP_OK) {
                valid=0u;
                ++sample_errors;
                ESP_LOGE(TAG,"TOUCH_SPI_ERROR err=%s errors=%u",
                         esp_err_to_name(spi),(unsigned)sample_errors);
            } else if(raw.x==0u || raw.y==0u ||
                      raw.x==4095u || raw.y==4095u ||
                      t02_map_d09_point(raw.x,raw.y,
                                    &x,&y)!=0) {
                valid=0u;
                ESP_LOGW(TAG,"TOUCH_MAP_REJECT raw=%u,%u",
                         (unsigned)raw.x,(unsigned)raw.y);
            }
        }
        if(down && !prev_down)
            ESP_LOGI(TAG,"TOUCH_DOWN raw=%u,%u mapped=%u,%u valid=%u triplet_x=%u,%u,%u triplet_y=%u,%u,%u",
                     (unsigned)raw.x,(unsigned)raw.y,(unsigned)x,(unsigned)y,
                     (unsigned)valid,
                     (unsigned)raw.xs[0],(unsigned)raw.xs[1],
                     (unsigned)raw.xs[2],(unsigned)raw.ys[0],
                     (unsigned)raw.ys[1],(unsigned)raw.ys[2]);
        if(!down && prev_down) ESP_LOGI(TAG,"TOUCH_UP");
        prev_down=down;
        if (game.phase==WARSHIP_LIFECYCLE_COMBAT ||
            game.phase==WARSHIP_LIFECYCLE_RESULT) {
            WarshipCombatHit hit={WARSHIP_COMBAT_NO_HIT,0u,0u};
            const WarshipCombatStatus got=warship_combat_input_step(
                &combat_input,(uint8_t)down,valid,x,y,ms,
                combat.pending,combat.view,&hit);
            if(got!=WARSHIP_COMBAT_OK) {
                ESP_LOGE(TAG,"D10_TOUCH_INVALID status=%u",(unsigned)got);
                return;
            }
            if(hit.command!=WARSHIP_COMBAT_NO_HIT) {
                const uint8_t had_selection=public_view.battle.has_selection;
                const uint8_t old_x=public_view.battle.target_x;
                const uint8_t old_y=public_view.battle.target_y;
                const WarshipCombatStatus applied=warship_combat_apply(
                    &combat,&game,&public_view,hit,ms,rng_u32,NULL);
                ESP_LOGI(TAG,"D10_ACTION id=%u cell=%u,%u view=%u result=%u",
                         (unsigned)hit.command,(unsigned)hit.x,(unsigned)hit.y,
                         (unsigned)combat.view,(unsigned)applied);
                if(applied==WARSHIP_COMBAT_OK) {
                    ++action_count;
                    /* Bounded DMA regions; never reinit SPI/touch. */
                    err=(hit.command==WARSHIP_COMBAT_CELL ||
                         hit.command==WARSHIP_COMBAT_CANCEL) ?
                        draw_combat_selection(had_selection,old_x,old_y) :
                        draw_combat(0u);
                    if(err!=ESP_OK) {
                        ESP_LOGE(TAG,"D10_DRAW_STOP err=%s",esp_err_to_name(err));
                        memory_log("d10_draw_fault");
                        return;
                    }
                    if(combat.pending) memory_log("after_feu");
                } else if(applied==WARSHIP_COMBAT_GAME_ERROR) {
                    ESP_LOGE(TAG,"D10_CONTROLLER_ERROR");
                }
            }
            if(warship_combat_reveal_due(&combat,ms)) {
                /* B01 consumed every gesture while busy before this point.
                 * Entire public pair is drawn in one composite, then ACK.
                 * Any LCD fault stops without acknowledging the resolution.
                 */
                /* A touch may start while SPI display blocks polling.
                 * Require a fresh release/down after the visual ACK. */
                warship_combat_input_discard(&combat_input);
                err=draw_combat(1u);
                if(err!=ESP_OK) {
                    ESP_LOGE(TAG,"D10_PAIR_DRAW_FAIL_NO_ACK err=%s",
                             esp_err_to_name(err));
                    memory_log("d10_reveal_fault");
                    return;
                }
                WarshipLifecycleEvent emitted=WARSHIP_LIFECYCLE_EVENT_NONE;
                if(warship_combat_presented(&combat,&game,&emitted)!=WARSHIP_COMBAT_OK) {
                    ESP_LOGE(TAG,"D10_ACK_FAIL_AFTER_PRESENTATION");
                    return;
                }
                /* D11 consumes only the event emitted AFTER the LCD
                 * composite returned ESP_OK and D07 ACK succeeded.
                 * Do not perform NVS inside any DMA callback.
                 */
                d11_runtime_event(emitted);
                if(emitted!=WARSHIP_LIFECYCLE_EVENT_NONE)
                    memory_log("after_nvs_event");
                ESP_LOGI(TAG,
                    "D10_PAIR_VISIBLE_ACK round=%u human=%u,%u impact=%u ship=%u captain=%u,%u impact=%u ship=%u event=%u",
                    (unsigned)combat.pending_pair.round,
                    (unsigned)combat.pending_pair.human.x,
                    (unsigned)combat.pending_pair.human.y,
                    (unsigned)combat.pending_pair.human.impact,
                    (unsigned)combat.pending_pair.human.ship_id,
                    (unsigned)combat.pending_pair.captain.x,
                    (unsigned)combat.pending_pair.captain.y,
                    (unsigned)combat.pending_pair.captain.impact,
                    (unsigned)combat.pending_pair.captain.ship_id,
                    (unsigned)emitted);
                if(warship_lifecycle_observe(&game,&public_view)!=WARSHIP_LIFECYCLE_OK) {
                    ESP_LOGE(TAG,"D10_POST_ACK_OBSERVE_FAIL");
                    return;
                }
                memory_log("after_reveal");
                if(combat.reveals>=2u) memory_log("after_rounds");
            }
        } else {
            WarshipDeployAction action=WARSHIP_DEPLOY_NO_ACTION;
            const WarshipDeployStatus dispatch=warship_deploy_input_step(
                &input,(uint8_t)down,valid,x,y,ms,(uint8_t)game.phase,&action);
            if(dispatch!=WARSHIP_DEPLOY_OK) {
                ESP_LOGE(TAG,"TOUCH_DISPATCH_ERROR status=%u",(unsigned)dispatch);
                return;
            }
            if(action!=WARSHIP_DEPLOY_NO_ACTION) {
                const uint8_t was=game.phase;
                const WarshipDeployStatus applied=warship_deploy_apply(
                    &game,action,rng_u32,NULL,rng_u32,NULL);
                ESP_LOGI(TAG,"ACTION id=%u raw=%u,%u mapped=%u,%u from=%u to=%u result=%u",
                         (unsigned)action,(unsigned)raw.x,(unsigned)raw.y,
                         (unsigned)x,(unsigned)y,(unsigned)was,
                         (unsigned)game.phase,(unsigned)applied);
                if(applied==WARSHIP_DEPLOY_OK) {
                    ++action_count;
                    if(action==WARSHIP_DEPLOY_READY) {
                        warship_combat_init(&combat,&combat_input,ms);
                        ESP_LOGI(TAG,"D10_COMBAT_ENTER view=ATTAQUE technical_geometry=1");
                    }
                    err=draw_for_action(action);
                    if(err!=ESP_OK) {
                        ESP_LOGE(TAG,"LCD_REFRESH_STOP err=%s",esp_err_to_name(err));
                        memory_log("draw_fault");
                        return;
                    }
                    if(action==WARSHIP_DEPLOY_READY)
                        ESP_LOGI(TAG,"D10_COMBAT_READY D11=NVS_events_after_ACK; no ESP-NOW");
                    memory_log("after_action");
                }
            }
            }
        if(ms-last_mem>=10000u) {
            memory_log("running");
            ESP_LOGI(TAG,"COUNTERS actions=%u touch_spi_errors=%u",
                     (unsigned)action_count,(unsigned)sample_errors);
            last_mem=ms;
        }
        /* At most one bounded NVS retry per 10 seconds, outside LCD/SPI
         * rendering. Never busy-loop on a failing/uncertain commit. */
        d11_runtime_poll();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
