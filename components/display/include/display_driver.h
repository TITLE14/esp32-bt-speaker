#ifndef DISPLAY_DRIVER
#define DISPLAY_DRIVER

#include "esp_err.h"
#include <stdbool.h>

typedef enum : uint8_t {
    STATUS_BAR_AREA,
    BATTERY_AREA,
    FULLSCREEN_AREA
} screen_area;

typedef enum : uint8_t {
    RSSI_LEVEL_1_ICON,
    RSSI_LEVEL_2_ICON,
    RSSI_LEVEL_3_ICON,
    RSSI_LEVEL_4_ICON,
    RSSI_LEVEL_COUNT
} rssi_level_icon;

typedef enum : uint8_t {
    BT_DISCONNECTED_ICON,
    BT_CONNECTED_ICON
} bt_icon_state;

typedef enum : uint8_t {
    SPEAKER_OFF_ICON,
    SPEAKER_ON_ICON
} speaker_state_icon;

esp_err_t screen_init();
void display_draw_battery_level(uint8_t level);
void display_draw_bt(bt_icon_state state);
void display_draw_rssi(rssi_level_icon level);
void display_draw_speaker(speaker_state_icon state);
esp_err_t display_update(screen_area area);
#endif