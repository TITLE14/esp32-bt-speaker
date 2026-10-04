idf_component_register(
    SRCS "src/i2s_driver.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_i2s app_logger ring_buf esp_driver_gpio
)