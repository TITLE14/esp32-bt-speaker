#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_err.h"
#include "esp_avrc_api.h"
#include "esp_log.h"
#include <stdint.h>
#include "esp_a2dp_api.h"
#include "esp_gap_bt_api.h"

static void avrc_evt_cb (esp_avrc_tg_cb_event_t event, esp_avrc_tg_cb_param_t *param) {
    switch (event) {
        case ESP_AVRC_TG_SET_ABSOLUTE_VOLUME_CMD_EVT: {
            uint8_t volume = param->set_abs_vol.volume;
            //i need a logger to get done with the absolute volume cb
            //tmp solution is:
            ESP_LOGI("AVRC_CB", "New volume is:%u", volume);
            break;
        }
        default: return;
    }
}

static void a2dp_evt_cb (esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param) {
    switch (event) {
        case ESP_A2D_PROF_STATE_EVT: {
            if(param->a2d_prof_stat.init_state == ESP_A2D_INIT_SUCCESS) ESP_LOGI("A2DP_EVT", "a2dp has been inited");
            break;
        }
        default: return;
    }
}

static void audio_data_cb(esp_a2d_conn_hdl_t conn_hdl, esp_a2d_audio_buff_t *audio_buf) {
    static uint8_t i = 0;
    i++;
    if (i >= 100) {
        ESP_LOGI("AUDIO_CB", "Undecoded data has been received");
        i = 0;
    }
}

static void gab_evt_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    switch(event) {
        case ESP_BT_GAP_CONFIG_EIR_DATA_EVT: {
            if(param->config_eir_data.stat == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI("GAP_CB", "EIR has been set successfully");
            }
        }
        default: return;
    }
}

esp_err_t bt_setup(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret != ESP_OK) return ret;

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_err_t cont_init = esp_bt_controller_init(&bt_cfg);
    if (cont_init != ESP_OK) {ESP_LOGI("BT_SETUP", "cont_init"); return cont_init;}

    esp_err_t cont_en = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
    if (cont_en != ESP_OK) {ESP_LOGI("BT_SETUP", "cont_en"); return cont_en;}

    esp_err_t bd_init = esp_bluedroid_init();
    if (bd_init != ESP_OK) {ESP_LOGI("BT_SETUP", "bd_init"); return bd_init;}
    esp_err_t bd_enable = esp_bluedroid_enable();
    if (bd_enable != ESP_OK) {ESP_LOGI("BT_SETUP", "bd_enable"); return bd_enable;}

    esp_err_t avrc_init = esp_avrc_tg_init();
    if(avrc_init != ESP_OK) {ESP_LOGI("BT_SETUP", "avrc_init"); return avrc_init;}

    esp_avrc_rn_evt_cap_mask_t evt_set = {0};
    esp_avrc_rn_evt_bit_mask_operation(ESP_AVRC_BIT_MASK_OP_SET, &evt_set, ESP_AVRC_RN_VOLUME_CHANGE);
    esp_avrc_tg_set_rn_evt_cap(&evt_set);

    esp_err_t reg_cb = esp_avrc_tg_register_callback(&avrc_evt_cb);
    if(reg_cb != ESP_OK) {ESP_LOGI("BT_SETUP", "reg_cb"); return reg_cb;}

    esp_err_t a2d_reg_cb = esp_a2d_register_callback(&a2dp_evt_cb);
    if(a2d_reg_cb != ESP_OK) {ESP_LOGI("BT_SETUP", "a2d_reg_cb"); return a2d_reg_cb;}
    esp_a2d_audio_buff_t* audio_buf = esp_a2d_audio_buff_alloc(4096);

    esp_err_t a2d_sink_init = esp_a2d_sink_init();
    if(a2d_sink_init != ESP_OK) {ESP_LOGI("BT_SETUP", "a2d_sink_init"); return a2d_sink_init;}

    esp_err_t gap_reg_cb = esp_bt_gap_register_callback(gab_evt_cb);
    if(gap_reg_cb != ESP_OK) {ESP_LOGI("BT_SETUP", "gap_reg_cb"); return gap_reg_cb;}

    esp_err_t reg_audio_data_cb = esp_a2d_sink_register_audio_data_callback(audio_data_cb);
    if(reg_audio_data_cb != ESP_OK) {ESP_LOGI("BT_SETUP", "reg_audio_data_cb"); return reg_audio_data_cb;}

    esp_bt_cod_t cod = {0};
    cod.major = ESP_BT_COD_MAJOR_DEV_AV;
    cod.minor = 0x05; //there's no AV minor macros in the header, it must be 5 according to specs
    cod.service = ESP_BT_COD_SRVC_AUDIO;

    esp_err_t set_cod = esp_bt_gap_set_cod(cod, ESP_BT_SET_COD_ALL);
    if(set_cod != ESP_OK) {ESP_LOGI("BT_SETUP", "set_cod"); return set_cod;}

    esp_err_t set_dev_name = esp_bt_gap_set_device_name("TITLE's Speaker");
    if(set_dev_name != ESP_OK) {ESP_LOGI("BT_SETUP", "set_dev_name"); return set_dev_name;}
    esp_bt_eir_data_t eir = {
        .include_uuid = true,
        .flag = ESP_BT_EIR_FLAG_GEN_DISC,
        .manufacturer_len = 0,
        .p_manufacturer_data = NULL
    };

    esp_err_t conf_eir = esp_bt_gap_config_eir_data(&eir);
    if(conf_eir != ESP_OK) {ESP_LOGI("BT_SETUP", "res"); return conf_eir;}

    esp_err_t set_scan_mode = esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    if (set_scan_mode != ESP_OK) {ESP_LOGI("BT_SETUP", "set_scan_mode"); return set_scan_mode;}

    return ESP_OK;
}

