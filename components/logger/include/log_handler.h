#ifndef LOG_HANDLER_H
#define LOG_HANDLER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <string.h>

#define LOG_TAG_MAX_LENGTH 10
#define LOG_TEXT_MAX_LENGTH 118

extern QueueHandle_t app_log_queue;

typedef struct {
    char tag[LOG_TAG_MAX_LENGTH];
    char text[LOG_TEXT_MAX_LENGTH];
} log_msg;

#if LOG_LOCAL_LEVEL >= ESP_LOG_INFO
    #define APP_LOG(arg_tag, arg_text, ...) do {                                \
        if (app_log_queue != NULL) {                                            \
            log_msg msg;                                                        \
            strlcpy(msg.tag, arg_tag, LOG_TAG_MAX_LENGTH);                      \
            snprintf(msg.text, LOG_TEXT_MAX_LENGTH, arg_text, ##__VA_ARGS__);   \
            xQueueSend(app_log_queue, &msg, 0);                                 \
        }                                                                       \
    } while(0)
#else
    #define APP_LOG(arg_tag, arg_text, ...)
#endif

void logger_init(void);

#endif //LOG_HANDLER_H