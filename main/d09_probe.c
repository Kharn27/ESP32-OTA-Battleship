/* D09+D10 technical presentation, D11 post-ACK NVS event handoff. A
 * single task owns both SPI buses, lifecycle, DMA and NVS writes. No radio. */
#include "d09_probe.h"
#ifdef WARSHIP_T03_DIAGNOSTIC
#include "t03_probe.h"
#endif
#include "d11_runtime.h"
#include "warship_deploy_ui.h"
#include "warship_combat_ui.h"
#include "warship_journey.h"
#include "warship_inspection.h"
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
/* D13 CPU overlay exists only in combat_frame.ships[] while opt-in ATTACK.
 * No separate permanent CPU fleet cache anywhere in the renderer. */

/* Same app_main writer as D10+D11; no extra ledger or private game copy. */
static WarshipEventStatus persist_forfeit(void *ctx, WarshipLifecycleEvent event)
{
    (void)ctx;
    return d11_runtime_event(event);
}

static int rng_u32(void *ctx,uint32_t *out)
{
    (void)ctx;
    if(out==NULL)return -1;
    *out=esp_random();
    return 0;
}
/* Explicitly opt-in D13 reception fixture: real D01/D02/D05/D07
 * algorithms receive deterministic random values, NOT private fleet
 * data, fake verdicts or injected scores. ALL normal builds still use
 * esp_random(). Mandatory D11 isolated namespace on the test image. */
#ifdef WARSHIP_D13_CYD_SCENARIO
#ifndef WARSHIP_D11_NVS_BENCH
#error "D13 reception scenario requires isolated D11 bench NVS"
#endif
typedef struct {uint32_t values[5];unsigned next;} D13FleetRng;
static D13FleetRng d13_human={{0u,20u,40u,60u,80u},0u};
static D13FleetRng d13_captain={{10u,30u,50u,70u,90u},0u};
static int d13_fleet_rng(void *ctx,uint32_t *out)
{
    D13FleetRng *const r=(D13FleetRng *)ctx;
    if(out==NULL||r==NULL)return -1;
    *out=r->values[r->next++%5u];
    return 0;
}
static int d13_shot_rng(void *ctx,uint32_t *out)
{
    (void)ctx;
    if(out==NULL)return -1;
    *out=0u; /* CPU logic still sees ONLY public battle state */
    return 0;
}
#define D13_HUMAN_RNG d13_fleet_rng
#define D13_HUMAN_CTX (&d13_human)
#define D13_CPU_FLEET_RNG d13_fleet_rng
#define D13_CPU_FLEET_CTX (&d13_captain)
#define D13_SHOT_RNG d13_shot_rng
#else
#define D13_HUMAN_RNG rng_u32
#define D13_HUMAN_CTX NULL
#define D13_CPU_FLEET_RNG rng_u32
#define D13_CPU_FLEET_CTX NULL
#define D13_SHOT_RNG rng_u32
#endif

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
#ifdef WARSHIP_T03_DIAGNOSTIC
    int64_t pixel_us=0, lcd_us=0;
#endif
    for(int top=y; top<y+h; top+=(int)WARSHIP_LCD_BAND_ROWS) {
        const int rows=(y+h-top)<(int)WARSHIP_LCD_BAND_ROWS ?
                       (y+h-top):(int)WARSHIP_LCD_BAND_ROWS;
#ifdef WARSHIP_T03_DIAGNOSTIC
        const int64_t pixel_start=esp_timer_get_time();
#endif
        for(int r=0;r<rows;++r) {
            for(int col=0;col<w;++col) {
                const uint16_t color=(game.phase==WARSHIP_LIFECYCLE_COMBAT ||
                                      game.phase==WARSHIP_LIFECYCLE_CONFIRM_ABANDON ||
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
#ifdef WARSHIP_T03_DIAGNOSTIC
        pixel_us+=esp_timer_get_time()-pixel_start;
        const int64_t lcd_start=esp_timer_get_time();
#endif
        const esp_err_t err=warship_lcd_send(x,top,w,rows);
#ifdef WARSHIP_T03_DIAGNOSTIC
        lcd_us+=esp_timer_get_time()-lcd_start;
#endif
        if(err!=ESP_OK) {
            ESP_LOGE(TAG,"DRAW_FAIL region=%d,%d,%d,%d band_y=%d err=%s",
                     x,y,w,h,top,esp_err_to_name(err));
            return err; /* No buffer overwrite after unknown DMA fault. */
        }
        ++sent;
    }
    ESP_LOGI(TAG,"DRAW x=%d y=%d w=%d h=%d bands=%u us=%" PRId64,
             x,y,w,h,sent,esp_timer_get_time()-begin);
#ifdef WARSHIP_T03_DIAGNOSTIC
    ESP_LOGI(TAG,"T03_RASTER x=%d y=%d w=%d h=%d bands=%u"
             " pixel_us=%" PRId64 " lcd_us=%" PRId64,
             x,y,w,h,sent,pixel_us,lcd_us);
#endif
    return ESP_OK;
}
static esp_err_t draw_combat(uint8_t include_pending)
{
    if (warship_lifecycle_observe(&game,&public_view)!=WARSHIP_LIFECYCLE_OK ||
        warship_combat_make_frame(&public_view,&combat,include_pending,
                                  &combat_frame)!=WARSHIP_COMBAT_OK ||
        warship_inspection_overlay(&game,&public_view,&combat,
                                   &combat_frame)!=WARSHIP_INSPECTION_APPLIED) {
        warship_inspection_clear_frame(&combat_frame);
        ESP_LOGE(TAG,"D13_PUBLIC_FRAME_FAIL pending=%u",
                 (unsigned)include_pending);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG,"COMBAT_FRAME view=%u busy=%u reveal=%u round=%u terminal=%u alert=%u d13_available=%u d13_visible=%u",
             (unsigned)combat_frame.view,(unsigned)combat_frame.busy,
             (unsigned)include_pending,(unsigned)combat_frame.rounds,
             (unsigned)combat_frame.terminal,(unsigned)combat_frame.alert,
             (unsigned)combat_frame.reveal_available,
             (unsigned)combat_frame.reveal_visible);
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
    if (game.phase==WARSHIP_LIFECYCLE_COMBAT ||
        game.phase==WARSHIP_LIFECYCLE_CONFIRM_ABANDON ||
        game.phase==WARSHIP_LIFECYCLE_RESULT)
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
#ifdef WARSHIP_D13_CYD_SCENARIO
    ESP_LOGW(TAG,"D13_SCENARIO=ARMED deterministic_rng=1 NVS=warship_d11lab TEST_ONLY");
#else
    ESP_LOGI(TAG,"D13_SCENARIO=OFF normal_random_gameplay=1");
#endif
    ESP_LOGI(TAG,"D13_BUFFERS sizeof_captain_temp=%u sizeof_combat_frame=%u allocations=0",
             (unsigned)sizeof(WarshipFleet),
             (unsigned)sizeof combat_frame);
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
                            D13_HUMAN_RNG,D13_HUMAN_CTX,
                            D13_CPU_FLEET_RNG,D13_CPU_FLEET_CTX)!=WARSHIP_DEPLOY_OK) {
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
#ifdef WARSHIP_T03_DIAGNOSTIC
    ESP_LOGW(TAG,"T03_DIAGNOSTIC=ON before normal D09 screen; no second LCD init");
    err=t03_probe_compare();
    if(err!=ESP_OK) {
        ESP_LOGE(TAG,"T03_DIAGNOSTIC_FAIL_NO_RETRY err=%s",esp_err_to_name(err));
        memory_log("t03_fault");
        return;
    }
    memory_log("t03_complete");
#endif
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
            game.phase==WARSHIP_LIFECYCLE_CONFIRM_ABANDON ||
            game.phase==WARSHIP_LIFECYCLE_RESULT) {
            WarshipCombatHit hit={WARSHIP_COMBAT_NO_HIT,0u,0u};
            const WarshipCombatStatus got=warship_journey_input_step(
                &combat_input,&combat,(uint8_t)down,valid,x,y,ms,&hit);
            if(got!=WARSHIP_COMBAT_OK) {
                ESP_LOGE(TAG,"D10_TOUCH_INVALID status=%u",(unsigned)got);
                return;
            }
            if(hit.command==WARSHIP_COMBAT_INSPECT) {
                const WarshipInspectionStatus result=
                    warship_inspection_toggle(&game,&public_view,&combat,hit);
                if(result==WARSHIP_INSPECTION_APPLIED) {
                    ++action_count;
                    /* Wipe the last transient CPU overlay immediately,
                     * INCLUDING on SHOW->HIDE, before any TFT redraw. */
                    warship_inspection_clear_frame(&combat_frame);
                    warship_combat_input_discard(&combat_input);
                    err=draw_combat(0u);
                    if(err!=ESP_OK) {
                        ESP_LOGE(TAG,"D13_DRAW_STOP err=%s",
                                 esp_err_to_name(err));
                        memory_log("d13_draw_fault");
                        return;
                    }
                    ESP_LOGI(TAG,"D13_INSPECTION status=APPLIED requested=%u visible=%u view=%u",
                        (unsigned)public_view.reveal_requested,
                        (unsigned)public_view.reveal_visible,
                        (unsigned)combat.view);
                    memory_log("after_d13_toggle");
                } else if(result==WARSHIP_INSPECTION_GAME_ERROR ||
                          result==WARSHIP_INSPECTION_BAD_ARGUMENT) {
                    warship_inspection_clear_frame(&combat_frame);
                    ESP_LOGE(TAG,"D13_INSPECTION_REJECT status=%u",
                             (unsigned)result);
                }
            } else if(hit.command!=WARSHIP_COMBAT_NO_HIT) {
                const uint8_t had_selection=public_view.battle.has_selection;
                const uint8_t old_x=public_view.battle.target_x;
                const uint8_t old_y=public_view.battle.target_y;
                WarshipEventStatus ledger=WARSHIP_EVENT_NO_CHANGE;
                const uint8_t from=game.phase;
                const WarshipJourneyStatus nav=warship_journey_apply(
                    &game,&combat,hit,D13_CPU_FLEET_RNG,D13_CPU_FLEET_CTX,
                    persist_forfeit,NULL,&ledger);
                if(nav==WARSHIP_JOURNEY_DONE ||
                   nav==WARSHIP_JOURNEY_SCORE_UNCONFIRMED) {
                    ++action_count;
                    /* Invalidate any D13 CPU overlay on RESULT exit/replay,
                     * and also when changing any D07 navigation phase. */
                    warship_inspection_clear_frame(&combat_frame);
                    /* D12 never reuses the tap that changed phase, even
                     * if it remains physically down during TFT refresh. */
                    warship_combat_input_discard(&combat_input);
                    if(game.phase==WARSHIP_LIFECYCLE_HOME ||
                       game.phase==WARSHIP_LIFECYCLE_DEPLOY)
                        warship_deploy_input_discard(&input,ms);
                    ESP_LOGI(TAG,
                        "D12_NAV from=%u to=%u command=%u status=%u nvs_status=%u",
                        (unsigned)from,(unsigned)game.phase,
                        (unsigned)hit.command,(unsigned)nav,(unsigned)ledger);
                    if(nav==WARSHIP_JOURNEY_SCORE_UNCONFIRMED)
                        ESP_LOGE(TAG,
                            "D12_FORFEIT_NOT_CONFIRMED_NVS status=%u no_fake_credit",
                            (unsigned)ledger);
                    err=draw_for_action(WARSHIP_DEPLOY_START);
                    if(err!=ESP_OK) {
                        ESP_LOGE(TAG,"D12_DRAW_STOP err=%s",esp_err_to_name(err));
                        memory_log("d12_draw_fault");
                        return;
                    }
                    memory_log("after_d12_nav");
                } else if(nav==WARSHIP_JOURNEY_NOT_HANDLED) {
                    const WarshipCombatStatus applied=warship_combat_apply(
                        &combat,&game,&public_view,hit,ms,D13_SHOT_RNG,NULL);
                    ESP_LOGI(TAG,"D10_ACTION id=%u cell=%u,%u view=%u result=%u",
                             (unsigned)hit.command,(unsigned)hit.x,(unsigned)hit.y,
                             (unsigned)combat.view,(unsigned)applied);
                    if(applied==WARSHIP_COMBAT_OK) {
                        ++action_count;
                        /* Bounded DMA regions; no new pixel buffers. */
                        err=(hit.command==WARSHIP_COMBAT_CELL ||
                             hit.command==WARSHIP_COMBAT_CANCEL) ?
                            draw_combat_selection(had_selection,old_x,old_y) :
                            draw_combat(0u);
                        if(err!=ESP_OK) {
                            ESP_LOGE(TAG,"D10_DRAW_STOP err=%s",
                                     esp_err_to_name(err));
                            memory_log("d10_draw_fault");
                            return;
                        }
                        if(combat.pending) memory_log("after_feu");
                    } else if(applied==WARSHIP_COMBAT_GAME_ERROR) {
                        ESP_LOGE(TAG,"D10_CONTROLLER_ERROR");
                    }
                } else if(nav==WARSHIP_JOURNEY_GAME_ERROR ||
                          nav==WARSHIP_JOURNEY_BAD_ARGUMENT) {
                    ESP_LOGE(TAG,"D12_NAV_REJECT from=%u command=%u status=%u",
                             (unsigned)from,(unsigned)hit.command,(unsigned)nav);
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
                /* The LCD composite and D07 ACK have BOTH succeeded.
                 * Keep the observable D10 ACK log before any NVS call,
                 * outside all SPI/LCD DMA callbacks. */
                d11_runtime_event(emitted);
                if(emitted!=WARSHIP_LIFECYCLE_EVENT_NONE)
                    memory_log("after_nvs_event");
                if(warship_lifecycle_observe(&game,&public_view)!=WARSHIP_LIFECYCLE_OK) {
                    ESP_LOGE(TAG,"D10_POST_ACK_OBSERVE_FAIL");
                    return;
                }
                memory_log("after_reveal");
                if(combat.reveals>=2u) memory_log("after_rounds");
                if(game.phase==WARSHIP_LIFECYCLE_RESULT) {
                    /* The pair was fully visible and already acknowledged.
                     * Now expose REJOUER/ACCUEIL technical controls. */
                    err=draw_combat(0u);
                    if(err!=ESP_OK) {
                        ESP_LOGE(TAG,"D12_RESULT_DRAW_FAIL err=%s",
                                 esp_err_to_name(err));
                        memory_log("d12_result_draw_fault");
                        return;
                    }
                    memory_log("after_d12_result");
                }
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
                    &game,action,D13_HUMAN_RNG,D13_HUMAN_CTX,
                    D13_CPU_FLEET_RNG,D13_CPU_FLEET_CTX);
                ESP_LOGI(TAG,"ACTION id=%u raw=%u,%u mapped=%u,%u from=%u to=%u result=%u",
                         (unsigned)action,(unsigned)raw.x,(unsigned)raw.y,
                         (unsigned)x,(unsigned)y,(unsigned)was,
                         (unsigned)game.phase,(unsigned)applied);
                if(applied==WARSHIP_DEPLOY_OK) {
                    ++action_count;
                    if(action==WARSHIP_DEPLOY_READY) {
                        warship_inspection_clear_frame(&combat_frame);
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
