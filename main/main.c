#include "bt_setup.h"
#include "esp_log.h"
#include "log_handler.h"

void app_main(void) {
    logger_init();
    esp_err_t setup_code = bt_setup();
    ESP_LOGI("MAIN", "bt setup finished with exit code %i", setup_code);
}