idf_component_register(
    SRCS "src/ring_buf.c" "src/ring_watcher.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES "app_logger"
)