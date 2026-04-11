#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "log_handler.h"
#include "ring_buf.h"
#include <stdint.h>

static void ring_watcher_task(void* args) {
    while(1) {
        uint16_t load = ring_used();
        APP_LOG("RING_WATCHER", "Ring buffer load is: %d", load);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void ring_watcher_start(void) {
    xTaskCreate(ring_watcher_task, "ring_watcher_task", 2048, NULL, 4, NULL);
}