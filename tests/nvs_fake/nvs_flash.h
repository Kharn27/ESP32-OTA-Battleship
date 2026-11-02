#ifndef D08_FAKE_NVS_FLASH_H
#define D08_FAKE_NVS_FLASH_H
#include "esp_err.h"
esp_err_t nvs_flash_init(void);
/* Intentionally NO erase prototype or implementation. The NVS adapter
 * must never erase a partition as automatic error recovery. */
#endif
