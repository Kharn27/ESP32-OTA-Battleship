#ifndef D14_FAKE_ESP_LOG_H
#define D14_FAKE_ESP_LOG_H
#define ESP_LOGI(tag,...) do { (void)(tag); } while (0)
#define ESP_LOGE(tag,...) do { (void)(tag); } while (0)
#endif
