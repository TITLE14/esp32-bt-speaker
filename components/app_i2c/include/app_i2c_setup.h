#ifndef APP_I2C_SETUP_H
#define APP_I2C_SETUP_H

#include "driver/i2c_master.h"
#include "esp_err.h"

typedef enum {
    FUEL_GAUGE = 0,
    OLED_SCREEN = 1,
    DEV_COUNT = 2
} app_i2c_dev;

void app_i2c_set_dev_cfg(i2c_device_config_t cfg, app_i2c_dev dev);
esp_err_t app_i2c_setup();

#endif