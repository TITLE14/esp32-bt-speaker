idf_component_register(
    SRCS "main.c"
    PRIV_REQUIRES ring_buf app_bt app_logger app_i2s #[[temp!!!]] display
)