/* D14 correction: experiment with PWM instead of unconditional BL=100%.
 * ESP32-2432S028R classic, LCD_BL GPIO21, assumed active high;
 * physical polarity/comfort to be verified by PO on real hardware.
 * LEDC LOW_SPEED TIMER_0 CHANNEL_0 5kHz / 10 bits; one owner/task.
 * NO LEDC fade ISR, no task, no additional buffers.
 */
#include "warship_lcd_light.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

#ifndef WARSHIP_LCD_BACKLIGHT_PERCENT
#define WARSHIP_LCD_BACKLIGHT_PERCENT 40
#endif
#if WARSHIP_LCD_BACKLIGHT_PERCENT < 0 || WARSHIP_LCD_BACKLIGHT_PERCENT > 100
#error WARSHIP_LCD_BACKLIGHT_PERCENT must be between 0 and 100
#endif

#define LIGHT_GPIO GPIO_NUM_21
#define LIGHT_MODE LEDC_LOW_SPEED_MODE
#define LIGHT_TIMER LEDC_TIMER_0
#define LIGHT_CHANNEL LEDC_CHANNEL_0
#define LIGHT_FREQ_HZ 5000u
#define LIGHT_DUTY_MAX 1023u /* 10 bits; avoid 1024 overflow on ESP32 */

static const char *const TAG = "WARSHIP_BL";
static bool ledc_owned, emitting;
static uint8_t percent = (uint8_t)WARSHIP_LCD_BACKLIGHT_PERCENT;

/* Reclaim pin from LEDC on failure, including partial channel_config().
 * Setting GPIO level while LEDC owns its matrix output is NOT sufficient.
 * The pre-write biases the GPIO latch low before matrix reassignment.
 */
static esp_err_t gpio_force_low(void)
{
    (void)gpio_set_level(LIGHT_GPIO, 0);
    const gpio_config_t out = {
        .pin_bit_mask = 1ULL << LIGHT_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    const esp_err_t configured = gpio_config(&out);
    if (configured != ESP_OK) {
        ESP_LOGE(TAG, "FORCE_GPIO_LOW_CONFIG_FAIL err=%s",
                 esp_err_to_name(configured));
        return configured;
    }
    const esp_err_t set = gpio_set_level(LIGHT_GPIO, 0);
    if (set != ESP_OK)
        ESP_LOGE(TAG, "FORCE_GPIO_LOW_SET_FAIL err=%s",
                 esp_err_to_name(set));
    return set;
}

esp_err_t warship_lcd_light_force_off(void)
{
    emitting = false;
    if (ledc_owned) {
        /* GPIO output matrix belongs to LEDC: force idle level LOW through
         * LEDC itself. IDF 5.5.6 ledc_stop() disables sig_out_en and sets
         * idle_level=0, and a later ledc_update_duty() re-enables it.
         */
        const esp_err_t stopped = ledc_stop(LIGHT_MODE,LIGHT_CHANNEL,0u);
        if (stopped == ESP_OK) return ESP_OK;
        ESP_LOGE(TAG, "LEDC_STOP_FAIL err=%s fallback_gpio=1",
                 esp_err_to_name(stopped));
        /* If stop fails, take control of GPIO matrix and latch it LOW.
         * Disable future PWM use; LCD caller makes this error sticky.
         */
        ledc_owned = false;
        const esp_err_t gpio = gpio_force_low();
        return gpio == ESP_OK ? stopped : gpio;
    }
    return gpio_force_low();
}

esp_err_t warship_lcd_light_init(void)
{
    if (ledc_owned) return ESP_ERR_INVALID_STATE;

    /* Off is established before timer/channel (channel starts at duty 0). */
    esp_err_t err = gpio_force_low();
    if (err != ESP_OK) return err;

    const ledc_timer_config_t timer = {
        .speed_mode = LIGHT_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LIGHT_TIMER,
        .freq_hz = LIGHT_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    err = ledc_timer_config(&timer);
    if (err != ESP_OK) goto init_fail;

    const ledc_channel_config_t channel = {
        .gpio_num = LIGHT_GPIO,
        .speed_mode = LIGHT_MODE,
        .channel = LIGHT_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LIGHT_TIMER,
        .duty = 0u,
        .hpoint = 0,
    };
    err = ledc_channel_config(&channel);
    if (err != ESP_OK) goto init_fail;

    ledc_owned = true;
    err = warship_lcd_light_force_off();
    if (err != ESP_OK) return err;
    const uint32_t configured_hz = ledc_get_freq(LIGHT_MODE,LIGHT_TIMER);
    if (configured_hz == 0u) {
        (void)warship_lcd_light_force_off();
        return ESP_FAIL;
    }
    ESP_LOGI(TAG,"PWM_INIT gpio=21 timer=0 channel=0 mode=LOW_SPEED"
             " freq_hz_actual=%u duty_bits=10 default_percent=%u"
             " initial_state=OFF assumed_active_high=1",
             (unsigned)configured_hz,(unsigned)percent);
    return ESP_OK;

init_fail:
    /* An unsuccessful channel_config can still have routed the pin. */
    (void)gpio_force_low();
    ESP_LOGE(TAG, "PWM_INIT_FAIL err=%s off_requested=1",
             esp_err_to_name(err));
    return err;
}

esp_err_t warship_lcd_light_set(bool on)
{
    if (!ledc_owned) return ESP_ERR_INVALID_STATE;
    if (!on || percent == 0u) {
        const esp_err_t err = warship_lcd_light_force_off();
        if (err == ESP_OK)
            ESP_LOGI(TAG,"PWM_STATE state=OFF gpio=21 idle_level=0");
        return err;
    }
    const uint32_t duty =
        (LIGHT_DUTY_MAX * (uint32_t)percent + 50u) / 100u;
    esp_err_t err = ledc_set_duty(LIGHT_MODE,LIGHT_CHANNEL,duty);
    if (err != ESP_OK) goto pwm_fault;
    err = ledc_update_duty(LIGHT_MODE,LIGHT_CHANNEL);
    if (err != ESP_OK) goto pwm_fault;
    const uint32_t actual_freq = ledc_get_freq(LIGHT_MODE,LIGHT_TIMER);
    if (actual_freq == 0u) {
        err = ESP_FAIL;
        goto pwm_fault;
    }
    emitting = true;
    /* No claim of optical output measurement: these values are the duty
     * applied by the LEDC API and the frequency read from the timer.
     * LEDC get_duty can reflect the previous PWM cycle immediately after
     * update, so log the successfully configured duty instead.
     */
    ESP_LOGI(TAG,"PWM_STATE state=ON gpio=21 mode=LOW_SPEED"
             " freq_hz_actual=%u duty_configured=%u/1023"
             " percent_requested=%u assumed_active_high=1",
             (unsigned)actual_freq,(unsigned)duty,(unsigned)percent);
    return ESP_OK;

pwm_fault:
    ESP_LOGE(TAG,"PWM_UPDATE_FAIL err=%s stop_idle_low=1",
             esp_err_to_name(err));
    (void)warship_lcd_light_force_off();
    return err;
}

esp_err_t warship_lcd_light_set_percent(uint8_t requested_percent)
{
    if (requested_percent > 100u) return ESP_ERR_INVALID_ARG;
    if (!ledc_owned) return ESP_ERR_INVALID_STATE;
    const uint8_t previous = percent;
    percent = requested_percent;
    if (emitting) {
        const esp_err_t err = warship_lcd_light_set(percent != 0u);
        if (err != ESP_OK) {
            percent = previous;
            return err;
        }
    }
    ESP_LOGI(TAG,"PWM_TARGET percent=%u active=%u",
             (unsigned)percent,(unsigned)emitting);
    return ESP_OK;
}
