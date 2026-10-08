#include "esp_log.h"
#include "esp_log_level.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "portmacro.h"
#include <string.h>
#include "app_logger.h"

QueueHandle_t app_log_queue = NULL;

static void logger_task(void* arg) {
    while (1) {
        log_msg msg;
        if (xQueueReceive(app_log_queue, &msg, portMAX_DELAY) == pdTRUE) {
            switch (msg.level) {
                case ESP_LOG_DEBUG:
                    ESP_LOGD(msg.tag, "%s", msg.text);
                    break;
                case ESP_LOG_WARN:
                    ESP_LOGW(msg.tag, "%s", msg.text);
                    break;
                case ESP_LOG_ERROR:
                    ESP_LOGE(msg.tag, "%s", msg.text);
                    break;
                case ESP_LOG_INFO:
                    ESP_LOGI(msg.tag, "%s", msg.text);
                    break;
                default:
                    ESP_LOGI(msg.tag, "%s", msg.text);
                    break;
            }
        }
    }
}

void logger_init(void) {
#if LOG_LOCAL_LEVEL >= ESP_LOG_NONE
    app_log_queue = xQueueCreate(10, sizeof(log_msg));
    xTaskCreate(&logger_task, "logger_task", 4096, NULL, 3, NULL);
#endif
}