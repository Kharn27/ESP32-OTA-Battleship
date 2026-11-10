#ifndef D14_FAKE_GPIO_H
#define D14_FAKE_GPIO_H
#include <stdint.h>
#include "esp_err.h"
#define GPIO_NUM_21 21
#define GPIO_MODE_OUTPUT 1
#define GPIO_PULLUP_DISABLE 0
#define GPIO_PULLDOWN_DISABLE 0
#define GPIO_INTR_DISABLE 0
typedef struct {
    uint64_t pin_bit_mask;
    int mode, pull_up_en, pull_down_en, intr_type;
} gpio_config_t;
esp_err_t gpio_config(const gpio_config_t *config);
esp_err_t gpio_set_level(int gpio_num, uint32_t level);
#endif
