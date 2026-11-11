#ifndef D14_FAKE_LEDC_H
#define D14_FAKE_LEDC_H
#include <stdint.h>
#include "esp_err.h"
#define LEDC_LOW_SPEED_MODE 1
#define LEDC_TIMER_0 0
#define LEDC_CHANNEL_0 0
#define LEDC_TIMER_10_BIT 10
#define LEDC_AUTO_CLK 0
#define LEDC_INTR_DISABLE 0
typedef struct {
    int speed_mode, duty_resolution, timer_num;
    uint32_t freq_hz;
    int clk_cfg;
} ledc_timer_config_t;
typedef struct {
    int gpio_num, speed_mode, channel, intr_type, timer_sel;
    uint32_t duty;
    int hpoint;
} ledc_channel_config_t;
esp_err_t ledc_timer_config(const ledc_timer_config_t *config);
esp_err_t ledc_channel_config(const ledc_channel_config_t *config);
esp_err_t ledc_stop(int mode, int channel, uint32_t idle);
esp_err_t ledc_set_duty(int mode, int channel, uint32_t duty);
esp_err_t ledc_update_duty(int mode, int channel);
uint32_t ledc_get_freq(int mode, int timer);
#endif
