#include "app_i2c_setup.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "log_handler.h"
#include <stdint.h>
#include "ina219_driver.h"

//For details, see the official datasheet: https://www.ti.com/lit/ds/symlink/ina219.pdf

#define MAX_CURRENT     1.6f
#define R_SHUNT         0.1f
#define CURRENT_RANGE   32768.0f
#define CURRENT_LSB     (MAX_CURRENT / CURRENT_RANGE)
#define BUS_VOLTAGE_LSB 0.004f

#define ADC_CONVERSION_TIME_MS 35
#define MEASUREMENT_CYCLE_TIME_MS (ADC_CONVERSION_TIME_MS * 2) // Shunt conversion + bus conversion.

enum : uint8_t {
    CONFIG_REG_ADDR  = 0x00,
    SHUNT_REG_ADDR   = 0x01,
    VOLTAGE_REG_ADDR = 0x02,
    CURRENT_REG_ADDR = 0x04,
    CALIBR_REG_ADDR  = 0x05
};

enum : uint8_t {
    RST_SHIFT  = 15,
    BRNG_SHIFT = 13,
    PG_SHIFT   = 11,
    BADC_SHIFT = 7,
    SADC_SHIFT = 3,
    MODE_SHIFT = 0
};

enum : uint16_t {
    RST_MASK  = 0b0001 << RST_SHIFT,
    BRNG_MASK = 0b0001 << BRNG_SHIFT,
    PG_MASK   = 0b0011 << PG_SHIFT,
    BADC_MASK = 0b1111 << BADC_SHIFT,
    SADC_MASK = 0b1111 << SADC_SHIFT,
    MODE_MASK = 0b0111 << MODE_SHIFT
};

enum : uint8_t {
    RST_VAL  = 0x0, // Setting this bit to '1' generates a system reset that is the same as power-on reset.
    BRNG_VAL = 0x0, // 0 = 16V FSR. 1 = 32V FSR (default value).
    PG_VAL   = 0x2, // 0x2 = ±160 mV. Sets PGA gain and range. Note that the PGA defaults to ÷8 (320mV range).
    BADC_VAL = 0xE, // These bits adjust the Bus ADC resolution or set the number of samples used when
    SADC_VAL = 0xE, // averaging results for the Bus Voltage Register. 0xE = 64 samples = 34.05 ms per register.
    MODE_VAL = 0x3  // 0x3 = triggered. Selects continuous, triggered, or power-down mode of operation. These bits default to continuous shunt and bus measurement mode.
};

static const uint16_t INA219_DEF_CONFIG_VAL = 
    (RST_VAL  << RST_SHIFT)   |
    (BRNG_VAL << BRNG_SHIFT)  |
    (PG_VAL   << PG_SHIFT)    |
    (BADC_VAL <<  BADC_SHIFT) |
    (SADC_VAL <<  SADC_SHIFT) |
    (MODE_VAL <<  MODE_SHIFT);

static const uint16_t INA219_CALIB_VAL = (uint16_t)(0.04096f / (CURRENT_LSB * R_SHUNT));

static i2c_master_dev_handle_t dev_hadnle;

static const i2c_device_config_t dev_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = (0x45),
    .scl_speed_hz = 100000
};

esp_err_t ina219_init() {
    // Read/write operations require Big-Endian format
    APP_ESP_RET_AND_LOG_ON_ERROR(app_i2c_add_device(&dev_cfg, &dev_hadnle));
    APP_ESP_RET_AND_LOG_ON_ERROR(i2c_master_transmit(dev_hadnle,
        (uint8_t[]){CALIBR_REG_ADDR, INA219_CALIB_VAL >> 8, INA219_CALIB_VAL & 0xFF},
        3,
        -1));
    return ESP_OK;
}

ina219_pwr_data ina219_get_pwr_data(void) {
    ina219_pwr_data pwr_data = {0};
    uint16_t config = INA219_DEF_CONFIG_VAL;
    
    
    ESP_ERROR_CHECK(i2c_master_transmit(dev_hadnle, (uint8_t[]){CONFIG_REG_ADDR,
    config >> 8, config & 0xFF}, 3, -1));

    uint8_t raw_vol_input[2];
    uint8_t raw_cur_input[2];

    vTaskDelay(pdMS_TO_TICKS(MEASUREMENT_CYCLE_TIME_MS));

    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_hadnle, (uint8_t[]){VOLTAGE_REG_ADDR},
    1, raw_vol_input, sizeof(raw_vol_input), -1));
    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_hadnle, (uint8_t[]){CURRENT_REG_ADDR},
    1, raw_cur_input, sizeof(raw_cur_input), -1));

    config = (INA219_DEF_CONFIG_VAL & ~MODE_MASK) | (0b000 << MODE_SHIFT); // Power-down mode. Register values ​​remain

    ESP_ERROR_CHECK(i2c_master_transmit(dev_hadnle, (uint8_t[]){CONFIG_REG_ADDR,
    config >> 8, config & 0xFF}, 3, -1));

    uint16_t raw_voltage = (((uint16_t)raw_vol_input[1] | ((uint16_t)raw_vol_input[0]) << 8) >> 3);
    int16_t raw_current  = ((int16_t)raw_cur_input[0] << 8 | raw_cur_input[1]);

    pwr_data.current = raw_current * CURRENT_LSB;     //to amperes
    pwr_data.voltage = raw_voltage * BUS_VOLTAGE_LSB; //to volts

    return pwr_data;
}