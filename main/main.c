#include "app_bt_setup.h"
#include "freertos/idf_additions.h"
#include "log_handler.h"
#include "app_i2s_setup.h"
#include "ring_watcher.h"
#include "display_driver.h"
#include "app_i2c_setup.h"

void app_main(void) {
    logger_init();
    app_i2c_init();
    screen_init();
    app_i2s_setup();
    app_bt_setup();
    ring_watcher_start();

    while (true) {
        display_draw_charge(100);
        display_draw_bt(BT_DISCONNECTED_ICON);
        display_draw_rssi(RSSI_LEVEL_4_ICON);
        display_draw_speaker(SPEAKER_ON_ICON);
        display_update(FULLSCREEN_AREA);

        vTaskDelay(pdMS_TO_TICKS(1000));

        display_draw_charge(81);
        display_draw_bt(BT_CONNECTED_ICON);
        display_draw_rssi(RSSI_LEVEL_3_ICON);
        display_draw_speaker(SPEAKER_OFF_ICON);
        display_update(FULLSCREEN_AREA);

        vTaskDelay(pdMS_TO_TICKS(1000));

        display_draw_charge(4);
        display_draw_bt(BT_DISCONNECTED_ICON);
        display_draw_rssi(RSSI_LEVEL_2_ICON);
        display_draw_speaker(SPEAKER_ON_ICON);
        display_update(FULLSCREEN_AREA);

        vTaskDelay(pdMS_TO_TICKS(1000));

        display_draw_charge(40);
        display_draw_bt(BT_CONNECTED_ICON);
        display_draw_rssi(RSSI_LEVEL_1_ICON);
        display_draw_speaker(SPEAKER_OFF_ICON);
        display_update(FULLSCREEN_AREA);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}