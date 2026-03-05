idf_component_register(
    SRCS "src/bt_setup.c"
    INCLUDE_DIRS "include"
    REQUIRES bt nvs_flash
)