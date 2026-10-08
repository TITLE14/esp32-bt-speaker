#ifndef DISPLAY
#define DISPLAY

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
} bt_state_icon;

typedef enum : uint8_t {
    SPEAKER_OFF_ICON,
    SPEAKER_ON_ICON
} speaker_state_icon;

/**
 * @brief Initializes the SSD1315 display and draws the initial UI state.
 */
esp_err_t display_init();

/**
 * @brief Draws the battery-related percentage value on the display.
 *
 * The value is displayed within the 'BATTERY_AREA'
 *
 * @param level A number in the range from 0 to 100.
 */
void display_draw_battery_level(uint8_t level);

/**
 * @brief Draws the Bluetooth connection state icon.
 *
 * The icon is displayed within the 'STATUS_BAR_AREA'
 *
 * @param state Bluetooth state icon.
 */
void display_draw_bt(bt_state_icon state);

/**
 * @brief Draws the Bluetooth RSSI level icon.
 *
 * The icon is displayed within the 'STATUS_BAR_AREA'
 *
 * @param level One of the 'rssi_level_icon' values, except 'RSSI_LEVEL_COUNT'.
 */
void display_draw_rssi(rssi_level_icon level);

/**
 * @brief Draws the speaker playback state icon.
 *
 * The icon is displayed within the 'STATUS_BAR_AREA'
 *
 * @param state Speaker state icon.
 */
void display_draw_speaker(speaker_state_icon state);

/**
 * @brief Updates the specified area of the display from the framebuffer.
 *
 * @param area Screen area to update.
 */
esp_err_t display_update(screen_area area);

#endif