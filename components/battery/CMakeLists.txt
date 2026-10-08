idf_component_register(
    SRC_DIRS "src"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES app_i2c app_logger display power_monitor
)