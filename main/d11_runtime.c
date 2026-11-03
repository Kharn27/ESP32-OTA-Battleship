#include "d11_runtime.h"
#include "warship_event_bridge.h"
#include "warship_event_id_nvs.h"
#include "warship_palmares_nvs.h"

#include <inttypes.h>
#include <stdint.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *const TAG="D11";
static WarshipEventBridge bridge;
static WarshipPalmaresNvsContext score_nvs;
static WarshipEventIdNvsContext id_nvs;
static uint8_t active;
static int64_t next_retry_us;
#define RETRY_PERIOD_US INT64_C(10000000)

static const char *name(WarshipEventStatus s)
{
    switch(s) {
    case WARSHIP_EVENT_PENDING: return "PENDING";
    case WARSHIP_EVENT_APPLIED: return "APPLIED";
    case WARSHIP_EVENT_ALREADY_APPLIED: return "ALREADY_APPLIED";
    case WARSHIP_EVENT_NO_CHANGE: return "NO_CHANGE";
    case WARSHIP_EVENT_WRITE_UNCERTAIN: return "WRITE_UNCERTAIN";
    case WARSHIP_EVENT_RELOAD_REQUIRED: return "RELOAD_REQUIRED";
    case WARSHIP_EVENT_READ_ERROR: return "READ_ERROR";
    case WARSHIP_EVENT_CORRUPT: return "CORRUPT";
    case WARSHIP_EVENT_INCOMPATIBLE_VERSION: return "INCOMPATIBLE_VERSION";
    case WARSHIP_EVENT_OVERFLOW: return "OVERFLOW";
    case WARSHIP_EVENT_CONFLICT: return "CONFLICT";
    case WARSHIP_EVENT_BAD_EVENT: return "BAD_EVENT";
    default: return "BAD_ARGUMENT";
    }
}
static void report(const char *stage,WarshipEventStatus result)
{
    ESP_LOGI(TAG,
        "D11_EVENT stage=%s status=%s pending=%u pending_id=%" PRIu64
        " pending_type=%u reserved_id=%" PRIu64
        " score_esp_err=0x%x seq_esp_err=0x%x",
        stage,name(result),(unsigned)bridge.pending,bridge.pending_id,
        (unsigned)bridge.pending_event,bridge.reserved_id,
        (unsigned)score_nvs.last_error,(unsigned)id_nvs.last_error);
    if(!active)return;
    WarshipPalmaresView v;
    const WarshipEventStatus read=warship_event_bridge_read(&bridge,&v);
    if(read!=WARSHIP_EVENT_APPLIED) {
        ESP_LOGW(TAG,"D11_COUNTERS status=%s (no unconfirmed totals)",
                 name(read));
        return;
    }
    uint64_t last_id=0u;
    WarshipLifecycleEvent last_event=WARSHIP_LIFECYCLE_EVENT_NONE;
    const WarshipEventStatus last=
        warship_event_bridge_last(&bridge,&last_id,&last_event);
    if(last!=WARSHIP_EVENT_APPLIED) {
        ESP_LOGE(TAG,"D11_LAST_ID status=%s",name(last));
        return;
    }
    ESP_LOGI(TAG,
        "D11_COUNTERS games=%" PRIu32 " victories=%" PRIu32
        " defeats=%" PRIu32 " draws=%" PRIu32
        " voluntary_forfeits=%" PRIu32 " last_id=%" PRIu64
        " last_type=%u",
        v.games,v.victories,v.defeats,v.draws,v.voluntary_forfeits,
        last_id,(unsigned)last_event);
}
void d11_runtime_init(void)
{
#ifdef WARSHIP_D11_NVS_BENCH
    /* Explicitly built, isolated persistent NVS test (never product
     * scores). Its first boot credits one synthetic victory. Later boots
     * READ and DEDUP the same id/type; no second credit and no reset. */
    score_nvs.namespace_name="warship_d11lab";
    id_nvs.namespace_name="warship_d11lab";
    ESP_LOGW(TAG,"D11_BENCH=ARMED namespace=warship_d11lab NOT_REAL_PALMARES");
#else
    ESP_LOGI(TAG,"D11_BENCH=OFF namespace=warship normal_non_destructive");
#endif
    const esp_err_t err=warship_palmares_nvs_init();
    ESP_LOGI(TAG,"D11_NVS_INIT esp_err=0x%x",(unsigned)err);
    if(err!=ESP_OK) {
        ESP_LOGE(TAG,"D11_NVS_INIT_FAIL_NO_ERASE code=%s",
                 esp_err_to_name(err));
        report("init_fail",WARSHIP_EVENT_READ_ERROR);
        return;
    }
    const WarshipEventStatus opened=warship_event_bridge_open(
        &bridge,warship_palmares_nvs_backend(),&score_nvs,
        warship_event_id_nvs_backend(),&id_nvs);
    if(opened!=WARSHIP_EVENT_APPLIED) {
        ESP_LOGE(TAG,"D11_NVS_READ_FAIL_CLOSED status=%s score_err=0x%x seq_err=0x%x",
                 name(opened),(unsigned)score_nvs.last_error,
                 (unsigned)id_nvs.last_error);
        report("open_fail",opened);
        return;
    }
    active=1u;
    report("boot_read",opened);
#ifdef WARSHIP_D11_NVS_BENCH
    uint64_t last_id=0u;
    WarshipLifecycleEvent last=WARSHIP_LIFECYCLE_EVENT_NONE;
    if(warship_event_bridge_last(&bridge,&last_id,&last)==WARSHIP_EVENT_APPLIED &&
       last_id==0u && bridge.reserved_id==0u) {
        ESP_LOGW(TAG,"D11_BENCH_FIRST_FLASH: explicit isolated VICTORY id=1");
        const WarshipEventStatus s=warship_event_bridge_receive(
            &bridge,WARSHIP_LIFECYCLE_EVENT_VICTORY);
        report("bench_first_write",s);
        if(warship_event_bridge_has_pending(&bridge))
            next_retry_us=esp_timer_get_time()+RETRY_PERIOD_US;
    } else if(last_id==1u && last==WARSHIP_LIFECYCLE_EVENT_VICTORY) {
        const WarshipPalmaresStatus dedup=warship_palmares_apply(
            &bridge.ledger,1u,WARSHIP_LIFECYCLE_EVENT_VICTORY);
        ESP_LOGI(TAG,"D11_BENCH_REBOOT_DEDUP status=%u expected=%u",
                 (unsigned)dedup,(unsigned)WARSHIP_PALMARES_ALREADY_APPLIED);
        report("bench_reboot",dedup==WARSHIP_PALMARES_ALREADY_APPLIED?
            WARSHIP_EVENT_ALREADY_APPLIED:WARSHIP_EVENT_CONFLICT);
    } else {
        ESP_LOGW(TAG,"D11_BENCH_NONEMPTY_OR_PENDING no_new_event no_erase last=%" PRIu64,
                 last_id);
    }
#endif
}
void d11_runtime_event(WarshipLifecycleEvent event)
{
    if(event==WARSHIP_LIFECYCLE_EVENT_NONE)return;
    if(!active) {
        ESP_LOGE(TAG,"D11_EVENT_NOT_RECORDED event=%u NVS_UNAVAILABLE",
                 (unsigned)event);
        return;
    }
    const WarshipEventStatus s=warship_event_bridge_receive(&bridge,event);
    report("post_d07_ack",s);
    if(s!=WARSHIP_EVENT_APPLIED && s!=WARSHIP_EVENT_ALREADY_APPLIED)
        ESP_LOGE(TAG,"D11_EVENT_NOT_CONFIRMED event=%u state=%s",
                 (unsigned)event,name(s));
    if(warship_event_bridge_has_pending(&bridge))
        next_retry_us=esp_timer_get_time()+RETRY_PERIOD_US;
}
void d11_runtime_poll(void)
{
    if(!active || !warship_event_bridge_has_pending(&bridge))return;
    const int64_t now=esp_timer_get_time();
    if(now<next_retry_us)return;
    next_retry_us=now+RETRY_PERIOD_US;
    const WarshipEventStatus s=warship_event_bridge_retry(&bridge);
    report("bounded_retry",s);
    if(s!=WARSHIP_EVENT_APPLIED && s!=WARSHIP_EVENT_ALREADY_APPLIED)
        ESP_LOGW(TAG,"D11_PENDING_NOT_DURABLE status=%s",name(s));
}
