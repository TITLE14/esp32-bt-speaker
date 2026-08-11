idf_component_register(
    SRCS "src/app_bt_setup.c"
    INCLUDE_DIRS "include"
    REQUIRES bt nvs_flash app_logger ring_buf app_i2s display
)