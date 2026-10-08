#ifndef BT_SETUP_H
#define BT_SETUP_H

#include "esp_err.h"

/**
 * @brief Initializes Bluetooth Classic and configures the device as an A2DP Sink named "Wave".
 */
esp_err_t app_bt_setup(void);

#endif