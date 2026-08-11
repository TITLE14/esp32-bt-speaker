#ifndef INA219_DRIVER_H
#define INA219_DRIVER_H

#include "esp_err.h"

typedef struct {
    float voltage; // volts
    float current; // amperes
} ina219_pwr_data;

esp_err_t ina219_init();
ina219_pwr_data ina219_get_pwr_data(void);

#endif