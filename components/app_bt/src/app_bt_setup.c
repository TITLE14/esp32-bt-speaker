#include "freertos/idf_additions.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_err.h"
#include <stddef.h>
#include <stdint.h>
#include "esp_a2dp_api.h"
#include "esp_gap_bt_api.h"
#include "app_logger.h"
#include "ring_buf.h"
#include "ring_watcher.h"
#include "i2s_driver.h"
#include "display.h"

typedef struct {
    int8_t rssi_delta;
    rssi_level_icon icon;
} rssi_threshold;

static const rssi_threshold rssi_table[] = {
    { 0,   RSSI_LEVEL_4_ICON },
    {-15,  RSSI_LEVEL_3_ICON },
    {-30,  RSSI_LEVEL_2_ICON },
    {-128, RSSI_LEVEL_1_ICON } 
};

static portMUX_TYPE a2dp_state_mux = portMUX_INITIALIZER_UNLOCKED;

static esp_bd_addr_t master_bda = {0};
static volatile bool a2d_is_connected = false;

static void audio_data_cb(const uint8_t *buf, uint32_t len) {
    ring_push(buf, len);
}

static void a2dp_evt_cb (esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param) {
    switch (event) {
        case ESP_A2D_CONNECTION_STATE_EVT: {
            esp_a2d_connection_state_t state = param->conn_stat.state;
            switch (state) {
                case ESP_A2D_CONNECTION_STATE_DISCONNECTED: {
                    portENTER_CRITICAL(&a2dp_state_mux);

                    a2d_is_connected = false;
                    memset(master_bda, 0, sizeof(esp_bd_addr_t));

                    portEXIT_CRITICAL(&a2dp_state_mux);

                    display_draw_bt(BT_DISCONNECTED_ICON);
                    display_draw_rssi(RSSI_LEVEL_1_ICON);
                    display_request_update(STATUS_BAR_AREA);

                    APP_LOGD("A2DP_EVT", "State is: disconnected");
                    break;
                }
                case ESP_A2D_CONNECTION_STATE_CONNECTED: {
                    portENTER_CRITICAL(&a2dp_state_mux);

                    a2d_is_connected = true;
                    memcpy(master_bda, param->conn_stat.remote_bda, sizeof(esp_bd_addr_t));

                    portEXIT_CRITICAL(&a2dp_state_mux);

                    display_draw_bt(BT_CONNECTED_ICON);
                    display_request_update(STATUS_BAR_AREA);

                    APP_LOGD("A2DP_EVT", "State is: connected");
                    break;
                }
                default: APP_LOGD("A2DP_EVT", "Connection state is: %d", state);
            }
            break;
        }
        case ESP_A2D_AUDIO_CFG_EVT: {
            esp_a2d_cie_sbc_t sbc_cfg = param->audio_cfg.mcc.cie.sbc_info;
            i2s_samp_freq sf = __builtin_ctz(sbc_cfg.samp_freq);
            bool is_mono = (sbc_cfg.ch_mode & ESP_A2D_SBC_CIE_CH_MODE_MONO) ? true : false;
            i2s_configure(is_mono, sf);
            break;
        }
        case ESP_A2D_AUDIO_STATE_EVT: {
            esp_a2d_audio_state_t state = param->audio_stat.state;
            switch (state) {
                case ESP_A2D_AUDIO_STATE_SUSPEND: {
                    i2s_stop();
                    ring_watcher_stop();
                    display_draw_speaker(SPEAKER_OFF_ICON);
                    display_request_update(STATUS_BAR_AREA);
                    APP_LOGD("A2DP_EVT", "Audio state: suspended");
                    break;
                }
                case ESP_A2D_AUDIO_STATE_STARTED: {
                    i2s_start();
                    ring_watcher_start();
                    display_draw_speaker(SPEAKER_ON_ICON);
                    display_request_update(STATUS_BAR_AREA);
                    APP_LOGD("A2DP_EVT", "Audio state: started");
                    break;
                }
                default: APP_LOGD("A2DP_EVT", "Audio state is: %d", state);
            }
            break;
        }
        default: APP_LOGD("A2DP_EVT","evt: %d", event);
    }
}

static void gab_evt_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    switch(event) {
        case ESP_BT_GAP_READ_RSSI_DELTA_EVT: {
            int8_t rssi = param->read_rssi_delta.rssi_delta;

            for (size_t i = 0; i < RSSI_LEVEL_COUNT; ++i) {
                if(rssi >= rssi_table[i].rssi_delta) {
                    display_draw_rssi(rssi_table[i].icon);
                    break;
                }
            }
            display_request_update(STATUS_BAR_AREA);
            break;
        }
        default: return;
    }
}

static void rssi_getter_task(void* param) {
    esp_bd_addr_t addr;
    bool connected;

    while (1) {
        portENTER_CRITICAL(&a2dp_state_mux);

        connected = a2d_is_connected;
        memcpy(addr, master_bda, sizeof(addr));

        portEXIT_CRITICAL(&a2dp_state_mux);

        if(connected) {
            esp_bt_gap_read_rssi_delta(addr);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

esp_err_t app_bt_setup(void) {
    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bt_controller_init(&bt_cfg));

    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));

    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bluedroid_init_with_cfg(&bluedroid_cfg));
    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bluedroid_enable());

    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bt_gap_set_device_name("Wave"));
    esp_bt_eir_data_t eir = {
        .include_uuid = true,
        .flag = ESP_BT_EIR_FLAG_GEN_DISC,
        .manufacturer_len = 0,
        .p_manufacturer_data = NULL
    };

    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bt_gap_config_eir_data(&eir));
    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bt_gap_register_callback(gab_evt_cb));

    APP_ESP_RET_AND_LOG_ON_ERROR(esp_a2d_register_callback(&a2dp_evt_cb));
    APP_ESP_RET_AND_LOG_ON_ERROR(esp_a2d_sink_init());
    APP_ESP_RET_AND_LOG_ON_ERROR(esp_a2d_sink_register_data_callback(audio_data_cb));

    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE));

    xTaskCreate(rssi_getter_task, "RSSI_Getter", 2048, NULL, 3, NULL);

    return ESP_OK;
}