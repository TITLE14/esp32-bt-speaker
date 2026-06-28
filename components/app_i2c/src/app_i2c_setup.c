#include "app_i2c_setup.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "hal/i2c_types.h"
#include "log_handler.h"

static i2c_master_bus_handle_t bus_handle;
static i2c_device_config_t dev_configs[DEV_COUNT];
static i2c_master_dev_handle_t dev_handles[DEV_COUNT];

i2c_master_bus_config_t i2c_mst_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = I2C_NUM_0,
    .scl_io_num = GPIO_NUM_19,
    .sda_io_num = GPIO_NUM_21,
    .glitch_ignore_cnt = 7
};

void app_i2c_set_dev_cfg(i2c_device_config_t cfg, app_i2c_dev dev) {
    if(dev < DEV_COUNT) {
        dev_configs[dev] = cfg;
    }
}

//requires to be called after all the device cfgs have been assigned
esp_err_t app_i2c_setup() {
    APP_ESP_RET_AND_LOG_ON_ERROR(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
    for(int i = 0; i < DEV_COUNT; ++i) {
        APP_ESP_RET_AND_LOG_ON_ERROR(i2c_master_bus_add_device(bus_handle, &dev_configs[i], &dev_handles[i]));
    }
    return ESP_OK;
}