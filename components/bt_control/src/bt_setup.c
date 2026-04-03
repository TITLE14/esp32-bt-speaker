#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_err.h"
#include <stdint.h>
#include "esp_a2dp_api.h"
#include "esp_gap_bt_api.h"
#include "log_handler.h"

#define APP_ESP_RET_AND_LOG_ON_ERROR(call) do {                                     \
    esp_err_t __app_err_rc = (call);                                                \
    if (__app_err_rc != ESP_OK) {                                                   \
        APP_LOG("BT_SETUP", "%s failed with exit code %d", #call, __app_err_rc);    \
        return __app_err_rc;                                                        \
    }                                                                               \
} while (0)

void audio_data_cb(const uint8_t *buf, uint32_t len) {
    static uint8_t i = 0;
    i++;
    if (i >= 70) {
        APP_LOG("AUDIO_CB", "Audio data has been received");
        i = 0;
    }
}

static void a2dp_evt_cb (esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param) {
    switch (event) {
        case ESP_A2D_CONNECTION_STATE_EVT: {
            esp_a2d_connection_state_t state = param->conn_stat.state;
            switch (state) {
                case ESP_A2D_CONNECTION_STATE_DISCONNECTED: APP_LOG("A2DP_EVT", "State is: disconnected");break;
                case ESP_A2D_CONNECTION_STATE_CONNECTING: APP_LOG("A2DP_EVT", "State is: connecting"); break;
                case ESP_A2D_CONNECTION_STATE_CONNECTED: APP_LOG("A2DP_EVT", "State is: connected"); break;
                case ESP_A2D_CONNECTION_STATE_DISCONNECTING: APP_LOG("A2DP_EVT", "State is: disconnecting"); break;
                default: APP_LOG("A2DP_EVT", "Connection state is: %d", state);
            }
            break;
        }
        case ESP_A2D_AUDIO_STATE_EVT: {
            esp_a2d_audio_state_t state = param->audio_stat.state;
            switch (state) {
                case ESP_A2D_AUDIO_STATE_SUSPEND: APP_LOG("A2DP_EVT", "Audio state: suspended"); break;
                case ESP_A2D_AUDIO_STATE_STARTED: APP_LOG("A2DP_EVT", "Audio state: started"); break;
                default: APP_LOG("A2DP_EVT", "Audio state is: %d", state);
            }
            break;
        }
        case ESP_A2D_AUDIO_CFG_EVT: {
            APP_LOG("A2DP_EVT","Audio cfg: ch_num %d, samp_freq %d, block_len %d", param->audio_cfg.mcc.cie.sbc_info.ch_mode,
                param->audio_cfg.mcc.cie.sbc_info.samp_freq, param->audio_cfg.mcc.cie.sbc_info.block_len);
        break;
        }

        default: APP_LOG("A2DP_EVT","evt: %d", event);
    }
}


static void gab_evt_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    switch(event) {
        case ESP_BT_GAP_CONFIG_EIR_DATA_EVT: {
            if(param->config_eir_data.stat == ESP_BT_STATUS_SUCCESS) {
                APP_LOG("GAP_CB", "EIR has been set successfully");
            }
        }
        default: return;
    }
}

esp_err_t bt_setup(void) {

    APP_ESP_RET_AND_LOG_ON_ERROR(nvs_flash_init());

    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bt_controller_init(&bt_cfg));

    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT));

    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bluedroid_init_with_cfg(&bluedroid_cfg));
    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bluedroid_enable());

    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bt_gap_set_device_name("TITLE's Speaker"));
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

    APP_ESP_RET_AND_LOG_ON_ERROR(esp_a2d_sink_set_delay_value(1600));

    APP_ESP_RET_AND_LOG_ON_ERROR(esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE));

    return ESP_OK;
}