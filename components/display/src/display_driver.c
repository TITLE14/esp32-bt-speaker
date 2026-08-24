//the code below is for ssd1315 oled i2c screen

#include "driver/i2c_master.h"
#include "app_i2c_setup.h"
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "log_handler.h"
#include <stdint.h>
#include <string.h>
#include "glyphs.h"
#include "display_driver.h"

#define BRIGHTNESS 0x7F

#define PAGE_WIDTH 128
#define PAGE_COUNT 8
#define PAGE_TO_INDEX(page) ((page) * PAGE_WIDTH)
#define DIGIT_TO_OFFSET(digit) ((digit) * 115)

enum : uint8_t {
    CMD_MODE =         0x00,
    DATA_MODE =        0x40,

    MEM_ADDR_MODE =    0x20, //0b000000XX | 0b00 = Horizontal Addressing Mode, 0b01 = Vertical Addressing Mode, 0b10 = Page Addressing Mode (RESET)
    CHARGE_PUMP =      0x8D, //0bX0010X0X | 0x14 = 7.5V (RESET), 0x94 = 8.5V, 0x95 = 9.0V
    SEG_REMAP_MODE =   0xA1,
    SCAN_REMAP_MODE =  0xC8,
    CONTRAST_CONTROL = 0x81, //0bXXXXXXXX | RESET = 7Fh
    DISPLAY_ON =       0xAF,

    SET_COLUMN_ADDR =  0x21, //0b0XXXXXXX (start address), 0b0YYYYYYY (end address)
    SET_PAGE_ADDR =    0x22  //0b00000XXX (start address), 0b00000YYY (end address)
};

enum : uint8_t {
    BT_ICON_POS = 0,
    SPEAKER_ICON_POS = BT_WIDTH + 10,
    RSSI_ICON_POS = PAGE_WIDTH - RSSI_WIDTH
};

typedef struct {
    uint8_t cursor_cmds[4];
    uint16_t fb_offset;
    uint16_t data_size;
} screen_area_config_t;

static i2c_master_dev_handle_t dev_handle;

static const i2c_device_config_t dev_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = 0x3C,
    .scl_speed_hz = 100000
};

static uint8_t framebuffer[1024] = {0};

static const uint8_t init_cmds[] = {
    CMD_MODE,
    MEM_ADDR_MODE,   0x00, // Horizontal Addressing Mode
    CHARGE_PUMP,     0x14, // 7.5V
    SEG_REMAP_MODE,
    SCAN_REMAP_MODE,
    SET_COLUMN_ADDR, 0x00, 0x7F,
    CONTRAST_CONTROL, BRIGHTNESS,
    DISPLAY_ON
};

static const screen_area_config_t screen_areas[] = {
    [STATUS_BAR_AREA] = {
        .cursor_cmds  = {CMD_MODE, SET_PAGE_ADDR, 0x00, 0x01},
        .fb_offset    = PAGE_TO_INDEX(0),
        .data_size    = STATUS_HEIGHT * PAGE_WIDTH
    },
    [BATTERY_AREA] = {
        .cursor_cmds  = {CMD_MODE, SET_PAGE_ADDR, 0x03, 0x07},
        .fb_offset    = PAGE_TO_INDEX(3),
        .data_size    = BATTERY_HEIGHT * PAGE_WIDTH
    },
    [FULLSCREEN_AREA] = {
        .cursor_cmds  = {CMD_MODE, SET_PAGE_ADDR, 0x00, 0x07},
        .fb_offset    = 0,
        .data_size    = PAGE_COUNT * PAGE_WIDTH
    }
};

static void glyph_draw(glyph_offset offset, glyph_height height, glyph_width width, int start_point) {
    for (int i = 0; i < height; ++i, start_point += PAGE_WIDTH) {
        memcpy(&framebuffer[start_point], &DejaVu_Sans_Glyphs[offset + (i * width)], width);
    }
}

esp_err_t screen_init(){
    APP_ESP_RET_AND_LOG_ON_ERROR(app_i2c_add_device(&dev_cfg, &dev_handle));
    APP_ESP_RET_AND_LOG_ON_ERROR(i2c_master_transmit(dev_handle, init_cmds,
    sizeof(init_cmds), -1));

    memset(&framebuffer[PAGE_TO_INDEX(2)], 0x04, PAGE_WIDTH); // status bar line
    display_draw_bt(BT_DISCONNECTED_ICON);
    display_draw_speaker(SPEAKER_OFF_ICON);
    display_draw_rssi(RSSI_LEVEL_1_ICON);
    display_update(FULLSCREEN_AREA);
    return ESP_OK;
}

void display_draw_battery_level(uint8_t level) {
    const uint8_t indent = 5; //px
    uint8_t digit_count;
    glyph_offset glyph_to_draw[3];

    if(level < 100 && level > 9) {
        digit_count = 2;
        glyph_to_draw[0] = DIGIT_TO_OFFSET(level / 10);
        glyph_to_draw[1] = DIGIT_TO_OFFSET(level % 10);
        glyph_to_draw[2] = 0;
    } else if (level < 10) {
        digit_count = 1;
        glyph_to_draw[0] = DIGIT_TO_OFFSET(level);
    } else {
        digit_count = 3;
        glyph_to_draw[0] = ONE_OFFSET;
        glyph_to_draw[1] = ZERO_OFFSET;
        glyph_to_draw[2] = ZERO_OFFSET;
    }

    memset(&framebuffer[PAGE_TO_INDEX(3)], 0, BATTERY_HEIGHT * PAGE_WIDTH); // charge area nullifying
    
    int base_index = PAGE_TO_INDEX(3);
    for (int i = 0; i < digit_count; ++i) {
        glyph_draw(glyph_to_draw[i], BATTERY_HEIGHT, DIGIT_WIDTH, base_index);
        base_index += DIGIT_WIDTH + indent;
    }
    glyph_draw(PERCENT_OFFSET, BATTERY_HEIGHT, PERCENT_WIDTH, base_index);
}

void display_draw_bt(bt_icon_state state) {
    glyph_offset glyph = BT_OFF_OFFSET + ((BT_WIDTH * STATUS_HEIGHT) * state);
    glyph_draw(glyph, STATUS_HEIGHT, BT_WIDTH, BT_ICON_POS);
}

void display_draw_rssi(rssi_level_icon level) {
    glyph_offset glyph = RSSI1_OFFSET + ((RSSI_WIDTH * STATUS_HEIGHT) * level);
    glyph_draw(glyph, STATUS_HEIGHT, RSSI_WIDTH, RSSI_ICON_POS);
}

void display_draw_speaker(speaker_state_icon state) {
    glyph_offset glyph = SPEAKER_OFF_OFFSET + ((SPEAKER_WIDTH * STATUS_HEIGHT) * state);
    glyph_draw(glyph, STATUS_HEIGHT, SPEAKER_WIDTH, SPEAKER_ICON_POS);
}

esp_err_t display_update(screen_area area) {
    const screen_area_config_t* area_cfg = &screen_areas[area];
    i2c_master_transmit_multi_buffer_info_t data_buf[2] = {
        {.write_buffer = (uint8_t[]){DATA_MODE}, .buffer_size = 1},
        {.write_buffer = &framebuffer[area_cfg->fb_offset], .buffer_size = area_cfg->data_size}
    };

    APP_ESP_RET_AND_LOG_ON_ERROR(i2c_master_transmit(dev_handle,
        area_cfg->cursor_cmds, 4, -1));
    APP_ESP_RET_AND_LOG_ON_ERROR(i2c_master_multi_buffer_transmit(dev_handle, data_buf, 2, -1));
    
    return ESP_OK;
}