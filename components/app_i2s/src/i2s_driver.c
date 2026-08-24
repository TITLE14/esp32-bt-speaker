// For details, see the official technical reference manual: https://www.mousenr.lt/pdfdocs/ESP32-Tech_Reference.pdf?srsltid=AfmBOopIkAqyA6kA0CNbsgdPdZEKxojQCzrADQ81qtk5k257k95TWN6w

#include "hal/i2s_ll.h"
#include "soc/regi2c_apll.h"
#include "esp_private/regi2c_ctrl.h"
#include "esp_rom_lldesc.h"
#include "soc/i2s_struct.h"
#include <stdbool.h>
#include "i2s_driver.h"

void i2s_init(bool is_mono) {
    //I2S_CONF_REG
    i2s_ll_tx_reset_fifo(&I2S0);
    i2s_ll_tx_enable_msb_shift(&I2S0, true);
    i2s_ll_tx_set_ws_width(&I2S0, 16);
    //I2S_CONF_CHAN_REG
    i2s_ll_tx_select_std_slot(&I2S0, I2S_STD_SLOT_BOTH, is_mono);
    
}