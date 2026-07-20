#ifndef APP_I2C_SETUP_H
#define APP_I2C_SETUP_H

#include "driver/i2c_master.h"
#include "esp_err.h"

esp_err_t app_i2c_add_device(const i2c_device_config_t *dev_config, i2c_master_dev_handle_t *ret_handle);
esp_err_t app_i2c_init();

#endif