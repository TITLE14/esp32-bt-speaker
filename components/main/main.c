#include "nvs_flash.h"
#include "app_bt_setup.h"
#include "i2s_driver.h"
#include "app_logger.h"
#include "ring_watcher.h"
#include "display.h"
#include "app_i2c_setup.h"
#include "battery_level.h"
#include "ring_buf.h"

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    logger_init();
    ring_init();
    ESP_ERROR_CHECK(app_i2c_init());
    ESP_ERROR_CHECK(display_init());
    battery_init();
    i2s_init();
    ESP_ERROR_CHECK(app_bt_setup());
    ring_watcher_init(5, 20, 500);
}