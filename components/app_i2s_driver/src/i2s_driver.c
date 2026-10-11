// For details, see the official technical reference manual: https://www.mouser.lt/pdfdocs/ESP32-Tech_Reference.pdf

#include "esp_attr.h"
#include "esp_intr_alloc.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/i2s_ll.h"
#include "portmacro.h"
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
#include "app_logger.h"
#include "ring_buf.h"
#include "esp_private/gpio.h"

#define DMA_BUF_SIZE   1024
#define DMA_BUF_COUNT  10

#define START_REQUESTED  (1U << 0)
#define RING_BUF_READY   (1U << 1)

#define START_THRESHOLD (RING_SIZE / 2)

typedef struct {
    uint8_t sdm0; // The value is 0 ~ 255
    uint8_t sdm1; // The value is 0 ~ 255
    uint8_t sdm2; // The value is 0 ~ 63
    uint8_t odir; // The value is 0 ~ 31
} apll_cuefs;

static const apll_cuefs apll_cfgs[] = {
    [I2S_SAMP_RATE_48] = {
        .sdm0 = 40,
        .sdm1 = 15,
        .sdm2 = 7,
        .odir = 7
    },
    [I2S_SAMP_RATE_44] = {
        .sdm0 = 32,
        .sdm1 = 41,
        .sdm2 = 6,
        .odir = 7
    }
};

static DMA_ATTR lldesc_t dma_desc[DMA_BUF_COUNT] = {0};
static DMA_ATTR uint8_t  dma_buf[DMA_BUF_COUNT][DMA_BUF_SIZE] = {0};

static TaskHandle_t write_task_handle = NULL;
static TaskHandle_t start_task_handle = NULL;


static EventGroupHandle_t start_events = NULL;
static const EventBits_t start_requirements = START_REQUESTED | RING_BUF_READY;

static int16_t current_sdm0 = 0;
static apll_cuefs current_apll_cfg = {0};

static lldesc_t* cpu_desc = &dma_desc[0];

static void IRAM_ATTR i2s_dma_isr(void *arg) {
    uint32_t status = i2s_ll_get_intr_status(&I2S0);

    if (status & I2S_LL_EVENT_TX_EOF) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        I2S0.int_clr.out_eof = 1;

        vTaskNotifyGiveFromISR(write_task_handle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static void i2s_starter(void* args) {
    while (1) {
        xEventGroupWaitBits(start_events,start_requirements,
            pdTRUE, pdTRUE, portMAX_DELAY);

        i2s_ll_start_out_link(&I2S0);
        i2s_ll_tx_start(&I2S0);
        APP_LOGD("I2S", "TX started");
    }
}

static void i2s_write_task(void* args) {
    uint8_t read_buf[DMA_BUF_SIZE];
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        while (cpu_desc->owner == 0) {

            if (ring_used() >= DMA_BUF_SIZE) {
                ring_read(read_buf, DMA_BUF_SIZE);
                memcpy((void*)cpu_desc->buf, read_buf, DMA_BUF_SIZE);
            } else {
                APP_LOGW("I2S", "DMA buf nullified! ring=%" PRIu32, ring_used());
                memset((void*)cpu_desc->buf, 0, DMA_BUF_SIZE);
            }

            cpu_desc->owner = 1;
            cpu_desc = cpu_desc->qe.stqe_next;
        }
    }
}

void i2s_init() {
    periph_module_enable(PERIPH_I2S0_MODULE);
    //I2S_CONF2_REG & I2S_PDM_CONF_REG
    i2s_ll_tx_enable_std(&I2S0);
    //I2S_SAMPLE_RATE_CONF_REG
    i2s_ll_tx_set_bits_mod(&I2S0, 16);
    i2s_ll_tx_set_bck_div_num(&I2S0, 4);
    i2s_ll_enable_dma(&I2S0, true);
    //I2S_CLKM_CONF_REG
    i2s_ll_tx_clk_set_src(&I2S0, I2S_CLK_SRC_APLL);
    i2s_ll_set_raw_mclk_div(&I2S0, 4, 1, 0);
    //I2S_LC_CONF_REG
    i2s_ll_dma_enable_auto_write_back(&I2S0, true);
    i2s_ll_dma_enable_owner_check(&I2S0, true);
    //I2S_CONF_REG
    i2s_ll_tx_enable_msb_shift(&I2S0, true);
    i2s_ll_tx_set_ws_width(&I2S0, 16);
    //I2S_FIFO_CONF_REG
    i2s_ll_tx_force_enable_fifo_mod(&I2S0, true);

    gpio_matrix_output(GPIO_NUM_22, I2S0O_BCK_OUT_IDX, false, false);
    gpio_matrix_output(GPIO_NUM_23,   I2S0O_WS_OUT_IDX,  false, false);
    gpio_matrix_output(GPIO_NUM_21, I2S0O_DATA_OUT23_IDX, false, false);
    
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

    start_events = xEventGroupCreate();
    xTaskCreate(i2s_write_task, "I2S_write", DMA_BUF_SIZE + 4096, NULL, 5, &write_task_handle);
    xTaskCreate(i2s_starter, "I2S_starter", 2048, NULL, 3, &start_task_handle);
    APP_LOGD("I2S", "Init complete");
}

void i2s_configure(bool is_mono, const i2s_samp_freq freq) {
    //I2S_CONF_REG
    i2s_ll_tx_reset(&I2S0);
    //I2S_CONF_CHAN_REG
    i2s_ll_tx_select_std_slot(&I2S0, I2S_STD_SLOT_BOTH, is_mono);
    //I2S_FIFO_CONF_REG
    i2s_ll_tx_enable_mono_mode(&I2S0, is_mono);
    //APLL configuration
    rtc_clk_apll_enable(true);
    current_apll_cfg = apll_cfgs[freq];
    rtc_clk_apll_coeff_set(current_apll_cfg.odir, current_apll_cfg.sdm0, current_apll_cfg.sdm1, current_apll_cfg.sdm2);
    current_sdm0 = current_apll_cfg.sdm0;

    APP_LOGD("I2S", "Configuration complete, mono=%d, sf=%d", is_mono, freq);
}

void i2s_start(void) {
    cpu_desc = &dma_desc[0];

    for (int i = 0; i < DMA_BUF_COUNT; ++i) {
        memset(dma_buf[i], 0, DMA_BUF_SIZE);
        dma_desc[i].owner = 1;
    }

    i2s_ll_tx_reset(&I2S0);
    i2s_ll_tx_reset_fifo(&I2S0);
    i2s_ll_tx_enable_intr(&I2S0);
    i2s_ll_set_out_link_addr(&I2S0, (uint32_t)&dma_desc & 0xFFFFF);
    regi2c_ctrl_write_reg(I2C_APLL, I2C_APLL_HOSTID, I2C_APLL_DSDM0, (uint8_t)current_apll_cfg.sdm0);

    xEventGroupSetBits(start_events, START_REQUESTED);
    ring_notify_on_fill(start_events, RING_BUF_READY, START_THRESHOLD);
    APP_LOGD("I2S", "Buffering");
}

void i2s_stop(void) {
    i2s_ll_tx_disable_intr(&I2S0);
    i2s_ll_tx_stop(&I2S0);
    i2s_ll_tx_stop_link(&I2S0);
    APP_LOGD("I2S", "TX stopped");
}

bool i2s_clock_tune(bool is_incr) {
    int16_t new_sdm0 = current_sdm0 + (is_incr ? 1 : -1);

    if (new_sdm0 < 0 || new_sdm0 > 255) {
        return false;
    }

    current_sdm0 = new_sdm0;
    regi2c_ctrl_write_reg(I2C_APLL, I2C_APLL_HOSTID, I2C_APLL_DSDM0, (uint8_t)current_sdm0);
    APP_LOGD("I2S", "New APLL SDM0 is %d", current_sdm0);
    return true;
}