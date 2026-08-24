#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "log_handler.h"
#include "ring_buf.h"
#include <stdint.h>

static void ring_watcher_task(void* args) {
    uint32_t min = UINT32_MAX;
    uint32_t max = 0;
    uint64_t sum = 0;
    uint32_t samples = 0;
    uint32_t first = 0;

    while (1) {
        uint32_t used = ring_used();

        if (samples == 0)
            first = used;

        if (used < min)
            min = used;

        if (used > max)
            max = used;

        sum += used;
        samples++;

        if (samples >= 50) {
            uint32_t avg = sum / samples;

            APP_LOG("RING",
                "first=%" PRIu32 " last=%" PRIu32 " min=%" PRIu32 " avg=%" PRIu32 " max=%" PRIu32,
                first, used, min, avg, max);

            min = UINT32_MAX;
            max = 0;
            sum = 0;
            samples = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void ring_watcher_start(void) {
    xTaskCreate(ring_watcher_task, "ring_watcher_task", 2048, NULL, 4, NULL);
}