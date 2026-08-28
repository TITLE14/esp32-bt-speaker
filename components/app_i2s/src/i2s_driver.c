// For details, see the official technical reference manual: https://www.mousenr.lt/pdfdocs/ESP32-Tech_Reference.pdf?srsltid=AfmBOopIkAqyA6kA0CNbsgdPdZEKxojQCzrADQ81qtk5k257k95TWN6w

#include "esp_attr.h"
#include "esp_intr_alloc.h"
#include "freertos/idf_additions.h"
#include "hal/i2s_ll.h"
#include "portmacro.h"
#include "rom/gpio.h"
#include "soc/gpio_num.h"
#include "soc/gpio_sig_map.h"
#include "soc/interrupts.h"
#include "soc/regi2c_apll.h"
#include "esp_private/regi2c_ctrl.h"
#include "soc/rtc.h"
#include "esp_rom_lldesc.h"
#include "soc/i2s_struct.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "i2s_driver.h"
#include "log_handler.h"
#include "ring_buf.h"

typedef struct {
    uint8_t sdm0; // The value is 0 ~ 255
    uint8_t sdm1; // The value is 0 ~ 255
    uint8_t sdm2; // The value is 0 ~ 63
    uint8_t odir; // The value is 0 ~ 31
} apll_cuefs;

static const apll_cuefs apll_cfgs[] = {
    [I2S_SAMP_FREQ_48] = {
        .sdm0 = 40,
        .sdm1 = 15,
        .sdm2 = 7,
        .odir = 7
    },
    [I2S_SAMP_FREQ_44] = {
        .sdm0 = 32,
        .sdm1 = 41,
        .sdm2 = 6,
        .odir = 7
    },
    [I2S_SAMP_FREQ_32] = {
        .sdm0 = 76,
        .sdm1 = 166,
        .sdm2 = 6,
        .odir = 11
    },
    [I2S_SAMP_FREQ_16] = {
        .sdm0 = 76,
        .sdm1 = 166,
        .sdm2 = 6,
        .odir = 24
    },
};

static DMA_ATTR lldesc_t dma_desc[DMA_BUF_COUNT] = {0};
static DMA_ATTR uint8_t  dma_buf[DMA_BUF_COUNT][DMA_BUF_SIZE] = {0};

static TaskHandle_t write_task_handle = NULL;

static int16_t current_sdm0 = 0;

static void IRAM_ATTR i2s_dma_isr(void *arg) {
    uint32_t status = i2s_ll_get_intr_status(&I2S0);

    if (status & I2S_OUT_EOF_INT_ST) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        I2S0.int_clr.out_eof = 1;

        vTaskNotifyGiveFromISR(write_task_handle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static void i2s_write_task(void* args) {
    uint8_t read_buf[DMA_BUF_SIZE];
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        lldesc_t* eof_desc = NULL;
        i2s_ll_tx_get_eof_des_addr(&I2S0, (void*)&eof_desc);

        if (ring_used() < DMA_BUF_SIZE) {
            eof_desc->owner = 1;
            continue;
        }
        ring_read(read_buf, DMA_BUF_SIZE);

        void* write_buf_addr = (void*)eof_desc->buf;
        memcpy(write_buf_addr, read_buf, DMA_BUF_SIZE);
        eof_desc->owner = 1;
    }
}

void i2s_init(bool is_mono, const i2s_samp_freq freq) {
    //I2S_CONF_REG
    i2s_ll_tx_reset_fifo(&I2S0);
    i2s_ll_tx_enable_msb_shift(&I2S0, true);
    i2s_ll_tx_set_ws_width(&I2S0, 16);
    //I2S_CONF_CHAN_REG
    i2s_ll_tx_select_std_slot(&I2S0, I2S_STD_SLOT_BOTH, is_mono);
    //I2S_SAMPLE_RATE_CONF_REG
    i2s_ll_tx_set_bits_mod(&I2S0, 16);
    i2s_ll_tx_set_bck_div_num(&I2S0, 4);
    //I2S_FIFO_CONF_REG
    i2s_ll_tx_force_enable_fifo_mod(&I2S0, true);
    i2s_ll_tx_enable_mono_mode(&I2S0, is_mono);
    i2s_ll_enable_dma(&I2S0, true);
    //I2S_CLKM_CONF_REG
    i2s_ll_tx_clk_set_src(&I2S0, I2S_CLK_SRC_APLL);
    i2s_ll_set_raw_mclk_div(&I2S0, 4, 1, 0);
    //I2S_LC_CONF_REG
    i2s_ll_dma_enable_auto_write_back(&I2S0, true);
    i2s_ll_dma_enable_owner_check(&I2S0, true);
    //APLL init
    const apll_cuefs* cfg = &apll_cfgs[freq];
    rtc_clk_apll_enable(true);
    rtc_clk_apll_coeff_set(cfg->odir, cfg->sdm0, cfg->sdm1, cfg->sdm2);
    current_sdm0 = cfg->sdm0;
    //GPIO matrix
    gpio_matrix_out(GPIO_NUM_26, I2S0O_BCK_OUT_IDX, false, false);
    gpio_matrix_out(GPIO_NUM_25,   I2S0O_WS_OUT_IDX,  false, false);
    gpio_matrix_out(GPIO_NUM_22, I2S0O_DATA_OUT0_IDX, false, false);
    
    esp_intr_alloc(ETS_I2S0_INTR_SOURCE, ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL1,
        i2s_dma_isr, NULL, NULL);
    
    _Static_assert(DMA_BUF_SIZE <= 4095, "DMA_BUF_SIZE too large");
    for (int i = 0; i < DMA_BUF_COUNT; i++) {
        dma_desc[i].size         = DMA_BUF_SIZE;
        dma_desc[i].length       = DMA_BUF_SIZE;
        dma_desc[i].eof          = 1;
        dma_desc[i].owner        = 1;
        dma_desc[i].buf          = dma_buf[i];
        dma_desc[i].qe.stqe_next = &dma_desc[(i + 1) % DMA_BUF_COUNT];
    }

    xTaskCreate(i2s_write_task, "I2S_write", 2048, NULL, 5, &write_task_handle);
    APP_LOG("I2S", "Init complete");
}

void i2s_start(void) {
    i2s_ll_tx_reset_fifo(&I2S0);
    i2s_ll_tx_enable_intr(&I2S0);
    i2s_ll_set_out_link_addr(&I2S0, (uint32_t)&dma_desc & 0xFFFFF);

    i2s_ll_start_out_link(&I2S0);
    i2s_ll_tx_start(&I2S0);
    APP_LOG("I2S", "TX started");
}

void i2s_stop(void) {
    i2s_ll_tx_disable_intr(&I2S0);
    i2s_ll_tx_stop(&I2S0);
    i2s_ll_tx_stop_link(&I2S0);
    APP_LOG("I2S", "TX stopped");
}

bool i2s_apll_freq_step(bool is_incr) {
    int16_t new_sdm0 = current_sdm0 + (is_incr ? 1 : -1);

    if (new_sdm0 < 0 || new_sdm0 > 255) {
        return false;
    }

    current_sdm0 = new_sdm0;
    regi2c_ctrl_write_reg(I2C_APLL, I2C_APLL_HOSTID, I2C_APLL_DSDM0, (uint8_t)current_sdm0);
    APP_LOG("I2S", "New APLL SDM0 is %d", current_sdm0);

    return true;
}