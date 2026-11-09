/* D14: compile the ACTUAL IDF-facing backlight controller against
 * deterministic LEDC/GPIO fakes. Verify stop not gpio_set_level alone.
 * Each CTest case is a fresh process to reset private driver static state.
 */
#include "warship_lcd_light.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) {\
    fprintf(stderr,"D14 LEDC line %d: %s\n",__LINE__,#expr);\
    return 1; } } while (0)

static int gpio_owner=0, pin_level=1;
static unsigned gpio_config_calls,stop_calls,update_calls,set_calls;
static uint32_t duty_configured;
static int timer_err, channel_err, stop_err, duty_err, update_err;
static int zero_frequency;

const char *esp_err_to_name(esp_err_t err)
{
    (void)err;
    return "FAKE_ERROR";
}

esp_err_t gpio_config(const gpio_config_t *config)
{
    if (!config || config->pin_bit_mask != (1ULL << GPIO_NUM_21) ||
        config->mode != GPIO_MODE_OUTPUT) return ESP_ERR_INVALID_ARG;
    ++gpio_config_calls;
    gpio_owner=1;
    return ESP_OK;
}

esp_err_t gpio_set_level(int gpio_num,uint32_t level)
{
    if(gpio_num!=GPIO_NUM_21 || level>1u) return ESP_ERR_INVALID_ARG;
    /* When LEDC holds GPIO matrix, this API cannot extinguish its PWM. */
    if(gpio_owner) pin_level=(int)level;
    return ESP_OK;
}

esp_err_t ledc_timer_config(const ledc_timer_config_t *config)
{
    if (!config || config->speed_mode!=LEDC_LOW_SPEED_MODE ||
        config->timer_num!=LEDC_TIMER_0 ||
        config->duty_resolution!=LEDC_TIMER_10_BIT ||
        config->freq_hz!=5000u) return ESP_ERR_INVALID_ARG;
    return timer_err ? ESP_FAIL : ESP_OK;
}

esp_err_t ledc_channel_config(const ledc_channel_config_t *config)
{
    if (!config || config->gpio_num!=21 ||
        config->speed_mode!=LEDC_LOW_SPEED_MODE ||
        config->channel!=LEDC_CHANNEL_0 ||
        config->timer_sel!=LEDC_TIMER_0 ||
        config->duty!=0u) return ESP_ERR_INVALID_ARG;
    gpio_owner=0; /* LEDC now owns matrix. */
    pin_level=(channel_err ? 1 : 0); /* partial routing on failure */
    return channel_err ? ESP_FAIL : ESP_OK;
}

esp_err_t ledc_stop(int mode,int channel,uint32_t idle)
{
    if(mode!=LEDC_LOW_SPEED_MODE || channel!=LEDC_CHANNEL_0 ||
       idle!=0u) return ESP_ERR_INVALID_ARG;
    ++stop_calls;
    if (stop_err) return ESP_FAIL;
    if(!gpio_owner) pin_level=(int)idle;
    return ESP_OK;
}

esp_err_t ledc_set_duty(int mode,int channel,uint32_t duty)
{
    if(mode!=LEDC_LOW_SPEED_MODE || channel!=LEDC_CHANNEL_0 ||
       duty>1023u) return ESP_ERR_INVALID_ARG;
    ++set_calls;
    if (duty_err) return ESP_FAIL;
    duty_configured=duty;
    return ESP_OK;
}

esp_err_t ledc_update_duty(int mode,int channel)
{
    if(mode!=LEDC_LOW_SPEED_MODE || channel!=LEDC_CHANNEL_0)
        return ESP_ERR_INVALID_ARG;
    ++update_calls;
    if (update_err) return ESP_FAIL;
    gpio_owner=0;
    pin_level=(int)(duty_configured!=0u);
    return ESP_OK;
}

uint32_t ledc_get_freq(int mode,int timer)
{
    if(mode!=LEDC_LOW_SPEED_MODE || timer!=LEDC_TIMER_0)
        return 0;
    return zero_frequency ? 0u : 5000u;
}

int main(int argc,char **argv)
{
    CHECK(argc==2);
    const char *case_name=argv[1];
    if (!strcmp(case_name,"nominal")) {
        CHECK(warship_lcd_light_set(true)==ESP_ERR_INVALID_STATE);
        CHECK(warship_lcd_light_init()==ESP_OK);
        CHECK(pin_level==0 && stop_calls==1u && gpio_config_calls==1u);
        CHECK(warship_lcd_light_set_percent(101u)==ESP_ERR_INVALID_ARG);
        CHECK(warship_lcd_light_set(true)==ESP_OK);
        CHECK(pin_level==1 && duty_configured==409u && update_calls==1u);
        CHECK(warship_lcd_light_set_percent(60u)==ESP_OK);
        CHECK(pin_level==1 && duty_configured==614u && update_calls==2u);
        CHECK(warship_lcd_light_set(false)==ESP_OK);
        CHECK(pin_level==0 && stop_calls==2u);
        CHECK(warship_lcd_light_set_percent(100u)==ESP_OK);
        CHECK(pin_level==0 && update_calls==2u);
        CHECK(warship_lcd_light_set(true)==ESP_OK);
        CHECK(pin_level==1 && duty_configured==1023u);
        CHECK(warship_lcd_light_set_percent(0u)==ESP_OK);
        CHECK(pin_level==0 && stop_calls==3u);
        CHECK(warship_lcd_light_set(true)==ESP_OK);
        CHECK(pin_level==0 && stop_calls==4u);
        CHECK(warship_lcd_light_force_off()==ESP_OK);
        CHECK(pin_level==0);
    } else if (!strcmp(case_name,"init_timer_fail")) {
        timer_err=1;
        CHECK(warship_lcd_light_init()==ESP_FAIL);
        CHECK(pin_level==0 && gpio_owner==1);
        CHECK(warship_lcd_light_set(true)==ESP_ERR_INVALID_STATE);
    } else if (!strcmp(case_name,"init_channel_fail")) {
        channel_err=1;
        CHECK(warship_lcd_light_init()==ESP_FAIL);
        CHECK(pin_level==0 && gpio_owner==1 && gpio_config_calls>=2u);
        CHECK(warship_lcd_light_set(true)==ESP_ERR_INVALID_STATE);
    } else if (!strcmp(case_name,"stop_fail")) {
        CHECK(warship_lcd_light_init()==ESP_OK);
        CHECK(warship_lcd_light_set(true)==ESP_OK);
        CHECK(pin_level==1 && gpio_owner==0);
        stop_err=1;
        CHECK(warship_lcd_light_force_off()==ESP_FAIL);
        CHECK(pin_level==0 && gpio_owner==1);
        CHECK(warship_lcd_light_set(true)==ESP_ERR_INVALID_STATE);
    } else if (!strcmp(case_name,"set_duty_fail")) {
        CHECK(warship_lcd_light_init()==ESP_OK);
        CHECK(warship_lcd_light_set(true)==ESP_OK);
        duty_err=1;
        CHECK(warship_lcd_light_set_percent(70u)==ESP_FAIL);
        CHECK(pin_level==0 && stop_calls>=2u);
    } else if (!strcmp(case_name,"update_fail")) {
        CHECK(warship_lcd_light_init()==ESP_OK);
        CHECK(warship_lcd_light_set(true)==ESP_OK);
        update_err=1;
        CHECK(warship_lcd_light_set_percent(70u)==ESP_FAIL);
        CHECK(pin_level==0 && stop_calls>=2u);
    } else if (!strcmp(case_name,"frequency_fail")) {
        CHECK(warship_lcd_light_init()==ESP_OK);
        zero_frequency=1;
        CHECK(warship_lcd_light_set(true)==ESP_FAIL);
        CHECK(pin_level==0 && stop_calls>=2u);
    } else return 2;
    puts("D14 LEDC PASS");
    return 0;
}
