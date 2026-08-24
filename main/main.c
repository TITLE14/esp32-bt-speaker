#include "app_bt_setup.h"
#include "freertos/idf_additions.h"
#include "log_handler.h"
#include "app_i2s_setup.h"
#include "ring_watcher.h"
#include "display_driver.h"
#include "app_i2c_setup.h"
#include "battery_level.h"

void app_main(void) {
    logger_init();
    app_i2c_init();
    vTaskDelay(pdMS_TO_TICKS(500));
    screen_init();
    battery_init();
    
    app_bt_setup();
    ring_watcher_start();
}