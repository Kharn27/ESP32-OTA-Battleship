#include "esp_log.h"
#include "t02_probe.h"
#include "warship_core.h"

static const char *const TAG = "CYD_WARSHIP";

void app_main(void)
{
    char banner[WARSHIP_CORE_BANNER_CAPACITY];
    if (warship_core_banner(banner, sizeof banner) != WARSHIP_CORE_OK) {
        ESP_LOGE(TAG, "Boot banner error");
        return;
    }
    ESP_LOGI(TAG, "%s", banner);
    ESP_LOGW(TAG, "T02 DIAGNOSTIC PROBE, NOT A GAME OR CALIBRATED TOUCH UI");
    t02_probe_run();
}
