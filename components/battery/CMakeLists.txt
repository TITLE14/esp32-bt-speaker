idf_component_register(
    SRCS "src/ina219_driver.c" "src/battery_level.c"
    INCLUDE_DIRS "include"
    REQUIRES app_i2c app_logger display
)