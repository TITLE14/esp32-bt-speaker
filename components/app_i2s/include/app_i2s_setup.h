#ifndef APP_I2S_SETUP_H
#define APP_I2S_SETUP_H

#include <esp_a2dp_api.h>

esp_err_t app_i2s_enable();
esp_err_t app_i2s_disable();
esp_err_t app_i2s_setup();

#endif