idf_component_register(
    SRC_DIRS "src"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_driver_i2s app_logger ring_buf esp_driver_gpio
)