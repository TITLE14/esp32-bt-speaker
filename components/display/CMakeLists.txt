idf_component_register(
    SRCS "src/display_driver.c"
    INCLUDE_DIRS "include"
    REQUIRES app_i2c app_logger
)