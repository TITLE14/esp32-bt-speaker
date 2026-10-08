#ifndef APP_LOGGER_H
#define APP_LOGGER_H

#include "esp_log_level.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <string.h>
#include "esp_err.h"

#define LOG_TAG_MAX_LENGTH 30
#define LOG_TEXT_MAX_LENGTH 400

typedef struct {
    esp_log_level_t level;
    char tag[LOG_TAG_MAX_LENGTH];
    char text[LOG_TEXT_MAX_LENGTH];
} log_msg;

extern QueueHandle_t app_log_queue;

#define BASE_APP_LOG(arg_level, arg_tag, arg_text, ...) do {                \
    if (app_log_queue != NULL) {                                            \
        log_msg msg;                                                        \
        msg.level = arg_level;                                              \
        strlcpy(msg.tag, arg_tag, LOG_TAG_MAX_LENGTH);                      \
        snprintf(msg.text, LOG_TEXT_MAX_LENGTH, arg_text, ##__VA_ARGS__);   \
        xQueueSend(app_log_queue, &msg, 0);                                 \
    }                                                                       \
} while(0)

#if LOG_LOCAL_LEVEL >= ESP_LOG_ERROR
    #define APP_LOGE(tag, text, ...) BASE_APP_LOG(ESP_LOG_ERROR, tag, text, ##__VA_ARGS__)
#else
    #define APP_LOGE(tag, text, ...)
#endif

#if LOG_LOCAL_LEVEL >= ESP_LOG_WARN
    #define APP_LOGW(tag, text, ...) BASE_APP_LOG(ESP_LOG_WARN, tag, text, ##__VA_ARGS__)
#else
    #define APP_LOGW(tag, text, ...)
#endif

#if LOG_LOCAL_LEVEL >= ESP_LOG_INFO
    #define APP_LOGI(tag, text, ...) BASE_APP_LOG(ESP_LOG_INFO, tag, text, ##__VA_ARGS__)
#else
    #define APP_LOGI(tag, text, ...)
#endif

#if LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG
    #define APP_LOGD(tag, text, ...) BASE_APP_LOG(ESP_LOG_DEBUG, tag, text, ##__VA_ARGS__)
#else
    #define APP_LOGD(tag, text, ...)
#endif

#define APP_ESP_RET_AND_LOG_ON_ERROR(call) do {                                     \
    esp_err_t __app_err_rc = (call);                                                \
    if (__app_err_rc != ESP_OK) {                                                   \
        APP_LOGE("LOGGER", "%s failed with exit code %d", #call, __app_err_rc);     \
        return __app_err_rc;                                                        \
    }                                                                               \
} while (0)

void logger_init(void);

#endif //APP_LOGGER_H