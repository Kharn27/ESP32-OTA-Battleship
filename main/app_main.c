#include "esp_log.h"
#include "t02_probe.h"
#include "warship_core.h"
#include "warship_fleet.h"
#include "warship_battle.h"
#include "warship_captain.h"
#include "warship_solo.h"
#include "warship_lifecycle.h"

static const char *const TAG = "CYD_WARSHIP";

void app_main(void)
{
    char banner[WARSHIP_CORE_BANNER_CAPACITY];
    if (warship_core_banner(banner, sizeof banner) != WARSHIP_CORE_OK) {
        ESP_LOGE(TAG, "Boot banner error");
        return;
    }
    ESP_LOGI(TAG, "%s", banner);
    /* Link-time integration only; preserve the existing T02 hardware probe.
     * No battle session, target selection or CPU decision is started.
     */
    ESP_LOGI(TAG, "D01 fleet component linked: carrier length=%u",
             (unsigned)warship_fleet_ship_length(WARSHIP_SHIP_CARRIER));
    ESP_LOGI(TAG, "D02 battle component linked: invalid state fails closed=%u",
             (unsigned)warship_battle_phase(NULL));
    ESP_LOGI(TAG, "D03 captain component linked: null input rejected=%u",
             (unsigned)warship_captain_choose(NULL, NULL, NULL, NULL));
    /* D06 link-only smoke; do not begin a game on this T02 hardware probe. */
    ESP_LOGI(TAG, "D06 solo session linked: null init rejected=%u",
             (unsigned)warship_solo_init(NULL, NULL, NULL));
    /* D07 link-only proof; no game is initialized or played on T02. */
    ESP_LOGI(TAG, "D07 lifecycle linked: null init rejected=%u",
             (unsigned)warship_lifecycle_init(NULL));
    ESP_LOGW(TAG, "T02 DIAGNOSTIC PROBE, NOT A GAME OR CALIBRATED TOUCH UI");
    t02_probe_run();
}
