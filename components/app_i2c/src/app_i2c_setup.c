#include "app_i2c_setup.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "hal/i2c_types.h"
#include "log_handler.h"

static i2c_master_bus_handle_t bus_handle;

i2c_master_bus_config_t i2c_mst_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = I2C_NUM_0,
    .scl_io_num = GPIO_NUM_27,
    .sda_io_num = GPIO_NUM_26,
    .glitch_ignore_cnt = 7
};

esp_err_t app_i2c_init() {
    APP_ESP_RET_AND_LOG_ON_ERROR(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
    return ESP_OK;
}

//requires to be called after app_i2c_init
esp_err_t app_i2c_add_device(const i2c_device_config_t *dev_config, i2c_master_dev_handle_t *ret_handle) {
    APP_ESP_RET_AND_LOG_ON_ERROR(i2c_master_bus_add_device(bus_handle, dev_config, ret_handle));
    return ESP_OK;
}