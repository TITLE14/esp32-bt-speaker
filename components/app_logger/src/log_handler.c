#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"
#include <string.h>
#include "log_handler.h"

QueueHandle_t app_log_queue = NULL;

static void logger_task(void* arg) {
    while (1) {
        log_msg msg;
        xQueueReceive(app_log_queue, &msg, portMAX_DELAY);
        ESP_LOGI(msg.tag, "%s", msg.text);
    }
}

void logger_init(void) {
    app_log_queue = xQueueCreate(10, sizeof(log_msg));
    xTaskCreate(&logger_task, "logger_task", 2048, NULL, 3, NULL);
}