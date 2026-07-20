#include "app_bt_setup.h"
#include "log_handler.h"
#include "app_i2s_setup.h"
#include "ring_watcher.h"
#include "display_driver.h"
#include "app_i2c_setup.h"

void app_main(void) {
    logger_init();
    app_i2s_setup();
    app_bt_setup();
    ring_watcher_start();
    app_i2c_init();
    screen_init();

    test_xfer();
}