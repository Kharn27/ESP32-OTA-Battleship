#ifndef WARSHIP_PALMARES_NVS_H
#define WARSHIP_PALMARES_NVS_H

#include "esp_err.h"
#include "warship_palmares.h"

#ifdef __cplusplus
extern "C" {
#endif

/* NVS namespace "warship", one blob key "palmares".
 * Caller owns the diagnostics context, which must outlive the ledger.
 * No erase, factory reset, migration or automatic flash recovery.
 * No reading or writing takes place until init/open/apply are invoked.
 */
typedef struct {
    esp_err_t last_error;
    /* NULL selects the production namespace warship. A dedicated bench
     * can opt into warship_d11lab WITHOUT altering production scores. */
    const char *namespace_name;
} WarshipPalmaresNvsContext;

/* Calls nvs_flash_init() ONLY; propagates errors unchanged, including
 * ESP_ERR_NVS_NO_FREE_PAGES / NEW_VERSION_FOUND. Never erases partitions. */
esp_err_t warship_palmares_nvs_init(void);

/* Inject these callbacks into warship_palmares_open(); the context must be
 * a non-NULL WarshipPalmaresNvsContext*. last_error retains ESP-IDF cause
 * of the most recent storage operation (ESP_OK on success / not found).
 */
WarshipPalmaresBackend warship_palmares_nvs_backend(void);

#ifdef __cplusplus
}
#endif
#endif
