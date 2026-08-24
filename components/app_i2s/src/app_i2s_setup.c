#include "driver/i2s_std.h"
#include "driver/i2s_common.h"
#include "driver/i2s_types.h"
#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "hal/i2s_types.h"
#include "log_handler.h"
#include "portmacro.h"
#include "ring_buf.h"
#include "soc/clk_tree_defs.h"
#include <esp_a2dp_api.h>
#include <stddef.h>
#include <stdint.h>

static i2s_chan_handle_t chan_handle;
static i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
static i2s_std_config_t std_cfg = {
    .clk_cfg = {
        .sample_rate_hz = 44100,
        .clk_src = I2S_CLK_SRC_APLL, 
        .mclk_multiple = I2S_MCLK_MULTIPLE_256, 
        .bclk_div = 8, 
    },
    .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
    .gpio_cfg = {
        .mclk = I2S_GPIO_UNUSED,
        .bclk = GPIO_NUM_26,
        .ws = GPIO_NUM_25,
        .dout = GPIO_NUM_22,
        .din = I2S_GPIO_UNUSED
    }
};

bool i2s_task_started = false;

esp_err_t app_i2s_setup() {
    chan_cfg.dma_frame_num = 1024;
    chan_cfg.dma_desc_num = 10;
    APP_ESP_RET_AND_LOG_ON_ERROR(i2s_new_channel(&chan_cfg, &chan_handle, NULL));

    APP_ESP_RET_AND_LOG_ON_ERROR(i2s_channel_init_std_mode(chan_handle, &std_cfg));

    return ESP_OK;
}

static void i2s_write_task(void* args) {
    static uint8_t buf[2048];

    while (1) {
        if(ring_used() < sizeof(buf)) {
            vTaskDelay(1);
            //TODO: notification from audio_data_cb
            continue;
        }
        ring_read(buf, sizeof(buf));
        size_t bytes_written;

        ESP_ERROR_CHECK(i2s_channel_write(chan_handle, buf, sizeof(buf), &bytes_written, portMAX_DELAY));

        ring_read_idx_incr(bytes_written);
    }
}

esp_err_t app_i2s_enable() {
    if(!i2s_task_started) {
        xTaskCreate(i2s_write_task, "i2s_write_task", 2048, NULL, 2, NULL);
        i2s_task_started = true;
    }
    APP_ESP_RET_AND_LOG_ON_ERROR(i2s_channel_enable(chan_handle));
    return ESP_OK;
}

esp_err_t app_i2s_disable() {
    APP_ESP_RET_AND_LOG_ON_ERROR(i2s_channel_disable(chan_handle));
    return ESP_OK;
}