idf_component_register(
    SRCS "src/bt_setup.c" "src/bt_audio_xfer.c"
    INCLUDE_DIRS "include"
    REQUIRES bt nvs_flash logger
)