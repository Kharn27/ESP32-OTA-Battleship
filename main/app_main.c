#include "esp_log.h"
#include "d09_probe.h"
#include "d11_runtime.h"
#include "warship_core.h"
#include "warship_fleet.h"
#include "warship_battle.h"
#include "warship_captain.h"
#include "warship_solo.h"
#include "warship_lifecycle.h"
#include "warship_palmares.h"
#include "warship_palmares_nvs.h"

static const char *const TAG = "CYD_WARSHIP";

void app_main(void)
{
    char banner[WARSHIP_CORE_BANNER_CAPACITY];
    if (warship_core_banner(banner, sizeof banner) != WARSHIP_CORE_OK) {
        ESP_LOGE(TAG, "Boot banner error");
        return;
    }
    ESP_LOGI(TAG, "%s", banner);
    /* Link-time checks only: D09 owns the sole hardware runtime below. */
    ESP_LOGI(TAG, "D01 fleet component linked: carrier length=%u",
             (unsigned)warship_fleet_ship_length(WARSHIP_SHIP_CARRIER));
    ESP_LOGI(TAG, "D02 battle component linked: invalid state fails closed=%u",
             (unsigned)warship_battle_phase(NULL));
    ESP_LOGI(TAG, "D03 captain component linked: null input rejected=%u",
             (unsigned)warship_captain_choose(NULL, NULL, NULL, NULL));
    /* D06 link-only smoke; no battle is started by this check. */
    ESP_LOGI(TAG, "D06 solo session linked: null init rejected=%u",
             (unsigned)warship_solo_init(NULL, NULL, NULL));
    /* D07 link-only proof; D09 initializes its technical deployment. */
    ESP_LOGI(TAG, "D07 lifecycle linked: null init rejected=%u",
             (unsigned)warship_lifecycle_init(NULL));
    /* D08 link-smoke below is NULL-only; D11 opens NVS explicitly later. */
    const WarshipPalmaresBackend storage = warship_palmares_nvs_backend();
    ESP_LOGI(TAG, "D08 NVS backend linked: %u",
             (unsigned)(storage.load != NULL && storage.store != NULL));
    /* Link all C99 ledger entry points without any storage operation:
     * NULL is rejected before the backend can read/write NVS. */
    ESP_LOGI(TAG, "D08 core link-only: open=%u read=%u apply=%u reload=%u",
             (unsigned)warship_palmares_open(NULL, storage, NULL),
             (unsigned)warship_palmares_read(NULL, NULL),
             (unsigned)warship_palmares_apply(NULL, 0u,
                                              WARSHIP_LIFECYCLE_EVENT_NONE),
             (unsigned)warship_palmares_reload(NULL));
    ESP_LOGW(TAG, "WARSHIP D09+D10 TECHNICAL + D11 PALMARES NVS EVENTS; NOT PRODUCT UI");
    ESP_LOGI(TAG, "BOOT_MODE=D09 direct boot, no Kconfig selector");
    /* D11 reads storage at app entry; no write/format on a normal boot. */
    d11_runtime_init();
    d09_probe_run();
}
