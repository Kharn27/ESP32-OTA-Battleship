#include "esp_log.h"
#include "warship_core.h"

static const char *const TAG = "CYD_WARSHIP";

void app_main(void)
{
    char banner[WARSHIP_CORE_BANNER_CAPACITY];

    if (warship_core_banner(banner, sizeof banner) != WARSHIP_CORE_OK) {
        ESP_LOGE(TAG, "Unable to prepare boot banner");
        return;
    }
    ESP_LOGI(TAG, "%s", banner);
    ESP_LOGI(TAG, "T01 firmware skeleton; display and touch are not initialized");
}
