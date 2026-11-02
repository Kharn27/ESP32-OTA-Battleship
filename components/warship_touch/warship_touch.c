#include "warship_touch.h"
#include <stddef.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

#define TOUCH_BUS SPI3_HOST
#define TOUCH_PIN_MOSI GPIO_NUM_32
#define TOUCH_PIN_MISO GPIO_NUM_39
#define TOUCH_PIN_SCLK GPIO_NUM_25
#define TOUCH_PIN_CS GPIO_NUM_33
#define TOUCH_PIN_IRQ GPIO_NUM_36
#define TOUCH_CLOCK_HZ 1000000

static const char *const TAG="WARSHIP_TOUCH";
static spi_device_handle_t dev;
static bool initialized;

esp_err_t warship_touch_init(void)
{
    if (initialized || dev != NULL) return ESP_ERR_INVALID_STATE;
    const gpio_config_t irq = {
        .pin_bit_mask = 1ULL << TOUCH_PIN_IRQ,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&irq);
    if (err != ESP_OK) return err;
    const spi_bus_config_t bus = {
        .mosi_io_num = TOUCH_PIN_MOSI,
        .miso_io_num = TOUCH_PIN_MISO,
        .sclk_io_num = TOUCH_PIN_SCLK,
        .quadwp_io_num = -1, .quadhd_io_num = -1, .max_transfer_sz = 3,
    };
    err = spi_bus_initialize(TOUCH_BUS,&bus,SPI_DMA_DISABLED);
    if (err != ESP_OK) return err;
    const spi_device_interface_config_t cfg = {
        .clock_speed_hz=TOUCH_CLOCK_HZ, .mode=0,
        .spics_io_num=TOUCH_PIN_CS, .queue_size=1,
    };
    err = spi_bus_add_device(TOUCH_BUS,&cfg,&dev);
    if (err != ESP_OK) {
        (void)spi_bus_free(TOUCH_BUS);
        ESP_LOGE(TAG,"TOUCH_INIT_FAIL err=%s",esp_err_to_name(err));
        return err;
    }
    initialized=true;
    ESP_LOGW(TAG,"SPI3 XPT2046 candidate MOSI=32 MISO=39 CLK=25 CS=33 IRQ=36 RAW_UNCALIBRATED=1");
    return ESP_OK;
}

bool warship_touch_pressed(void)
{
    return initialized && gpio_get_level(TOUCH_PIN_IRQ)==0;
}
static esp_err_t read_adc(uint8_t command,uint16_t *out)
{
    if (!initialized || dev==NULL || out==NULL)
        return ESP_ERR_INVALID_STATE;
    spi_transaction_t tx = {
        .length=24,
        .flags=SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
    };
    tx.tx_data[0]=command;
    esp_err_t err=spi_device_transmit(dev,&tx);
    if (err!=ESP_OK) return err;
    const uint16_t packed=((uint16_t)tx.rx_data[1]<<8)|tx.rx_data[2];
    *out=(packed>>3)&0x0fffu;
    return ESP_OK;
}
static uint16_t median3(uint16_t a,uint16_t b,uint16_t c)
{
    if (a>b) {const uint16_t t=a; a=b; b=t;}
    if (b>c) {const uint16_t t=b; b=c; c=t;}
    if (a>b) {const uint16_t t=a; a=b; b=t;}
    return b;
}
esp_err_t warship_touch_sample(WarshipTouchSample *out)
{
    if (!initialized || out==NULL) return ESP_ERR_INVALID_STATE;
    WarshipTouchSample sample={0};
    for(unsigned i=0u;i<3u;++i) {
        esp_err_t err=read_adc(0xd0u,&sample.xs[i]);
        if(err!=ESP_OK) return err;
        err=read_adc(0x90u,&sample.ys[i]);
        if(err!=ESP_OK) return err;
    }
    sample.x=median3(sample.xs[0],sample.xs[1],sample.xs[2]);
    sample.y=median3(sample.ys[0],sample.ys[1],sample.ys[2]);
    *out=sample;
    return ESP_OK;
}
