#ifndef POWER_MONITOR
#define POWER_MONITOR

#include "esp_err.h"

typedef struct {
    float voltage; // volts
    float current; // amperes
} pwr_data;

/**
 * @brief Initializes the power monitor.
 */
esp_err_t pw_init();

/**
 * @brief Performs a voltage and current measurement.
 *
 * @return Measured bus voltage and current converted to SI units.
 */
pwr_data pw_get_data(void);

#endif