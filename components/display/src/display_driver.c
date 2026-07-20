#include "driver/i2c_master.h"
#include "app_i2c_setup.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "log_handler.h"
#include <stdint.h>
#include <string.h>
#include "glyphs.h"

static i2c_master_dev_handle_t dev_handle;

static uint8_t framebuffer[1025] = {0x40};

i2c_device_config_t dev_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = (0x78 >> 1),
    .scl_speed_hz = 200000
};

esp_err_t screen_init(){
    app_i2c_add_device(&dev_cfg, &dev_handle);
    return ESP_OK;
}


uint8_t screen_init_buf[] = {
    0x00,
    0x20, 0x00,
    0x8D, 0x14,
    0xA1,
    0xC8,
    0x81, 0x7F,
    0xAF
};

uint8_t fb_cursor[] = {
    0x00,
    0x20,
    0x00,
    0x21,
    0x00,
    0x7F,
    0x22,
    0x00,
    0x07
};

uint8_t status_bar_line[128];


esp_err_t test_xfer(void) {
    APP_ESP_RET_AND_LOG_ON_ERROR(i2c_master_transmit(dev_handle, screen_init_buf,
    sizeof(screen_init_buf), -1));
    for (int i = 0; i < sizeof(status_bar_line); ++i) {
        status_bar_line[i] = 0x80;
    }

    memcpy(&framebuffer[129], status_bar_line, sizeof(status_bar_line));
    
    for (int i = 0, start_point = 257 + 128; i < CHARGE_HEIGHT; ++i, start_point += 128) {
        memcpy(&framebuffer[start_point], &DejaVu_Sans_Glyphs[EIGHT + (i * DIGIT_WIDTH)], DIGIT_WIDTH);
    }

    for (int i = 0, start_point = 287 + 128; i < CHARGE_HEIGHT; ++i, start_point += 128) {
        memcpy(&framebuffer[start_point], &DejaVu_Sans_Glyphs[SEVEN + (i * DIGIT_WIDTH)], DIGIT_WIDTH);
    }

    for (int i = 0, start_point = 317 + 128; i < CHARGE_HEIGHT; ++i, start_point += 128) {
        memcpy(&framebuffer[start_point], &DejaVu_Sans_Glyphs[PERCENT + (i * PERCENT_WIDTH)], PERCENT_WIDTH);
    }

    for (int i = 0, start_point = 60; i < STATUS_HEIGHT; ++i, start_point += 128) {
        for (int j = 0; j < SPEAKER_WIDTH; ++j) {
            framebuffer[start_point + j] |= DejaVu_Sans_Glyphs[SPEAKER_ON + (i * SPEAKER_WIDTH) + j];
        }
    }

    for (int i = 0, start_point = 80; i < STATUS_HEIGHT; ++i, start_point += 128) {
        for (int j = 0; j < BT_WIDTH; ++j) {
            framebuffer[start_point + j] |= DejaVu_Sans_Glyphs[BT_ON + (i * BT_WIDTH) + j];
        }
    }

    for (int i = 0, start_point = 100; i < STATUS_HEIGHT; ++i, start_point += 128) {
        for (int j = 0; j < RSSI_WIDTH; ++j) {
            framebuffer[start_point + j] |= DejaVu_Sans_Glyphs[RSSI2 + (i * RSSI_WIDTH) + j];
        }
    }
    

    APP_ESP_RET_AND_LOG_ON_ERROR(i2c_master_transmit(dev_handle, fb_cursor,
    sizeof(fb_cursor), -1));
    APP_ESP_RET_AND_LOG_ON_ERROR(i2c_master_transmit(dev_handle, framebuffer,
    sizeof(framebuffer), -1));
    return ESP_OK;
}