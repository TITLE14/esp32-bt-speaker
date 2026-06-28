idf_component_register(
    SRCS "src/app_i2c_setup.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_i2c app_logger
)