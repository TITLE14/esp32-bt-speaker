#include "app_bt_setup.h"
#include "esp_log.h"
#include "log_handler.h"
#include "app_i2s_setup.h"
#include "ring_watcher.h"

void app_main(void) {
    logger_init();
    app_i2s_setup();
    esp_err_t bt_setup_rc = app_bt_setup();
    ring_watcher_start();
    ESP_LOGI("MAIN", "bt setup finished with exit code %i", bt_setup_rc);
}