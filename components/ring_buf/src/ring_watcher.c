#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "app_logger.h"
#include "ring_buf.h"
#include <stdint.h>
#include "i2s_driver.h"
#include "ring_watcher.h"

static uint32_t high_threshold = 0;
static uint32_t low_threshold = 0;

static uint16_t samp_count = 0;
static uint16_t samp_interval = 0;

static volatile bool watcher_started = false;
static TaskHandle_t ring_watcher_handle = NULL;

static void ring_watcher_task(void* args) {
    static uint64_t used_sum = 0;
    static uint32_t used_avg = 0;

    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        APP_LOGD("RING_WATCHER", "started");
        while (watcher_started) {
            used_sum = 0;
            for (uint16_t i = 0; i < samp_count; ++i) {
                if (!watcher_started) {
                    break;
                }

                used_sum += ring_used();
                vTaskDelay(pdMS_TO_TICKS(samp_interval));
            }

            if (!watcher_started) {
                break;
            }

            used_avg = used_sum / samp_count;
            APP_LOGD("RING_WATCHER", "ring used avg=%" PRIu32, used_avg);

            if (used_avg > high_threshold) {
                i2s_clock_tune(1);
                continue;
            } else if (used_avg < low_threshold) {
                i2s_clock_tune(0);
                continue;
            }
        }
        APP_LOGD("RING_WATCHER", "stopped");
    }
}

void ring_watcher_start(void) {
    watcher_started = true;
    xTaskNotifyGive(ring_watcher_handle);
}

void ring_watcher_stop(void) {
    watcher_started = false;
}

void ring_watcher_init(uint8_t deadband_radius_percent, uint16_t sample_count, uint16_t sampling_interval_ms) {
    low_threshold =  (RING_SIZE / 2u) - (RING_SIZE * deadband_radius_percent / 100u);
    high_threshold = (RING_SIZE / 2u) + (RING_SIZE * deadband_radius_percent / 100u);

    samp_count = sample_count;
    samp_interval = sampling_interval_ms;
    xTaskCreate(ring_watcher_task, "ring_watcher_task", 4096, NULL, 4, &ring_watcher_handle);
}