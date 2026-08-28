#ifndef I2S_DRIVER_H
#define I2S_DRIVER_H

#include <stdbool.h>

#define DMA_BUF_SIZE   2048
#define DMA_BUF_COUNT  2

typedef enum {
    I2S_SAMP_FREQ_48,
    I2S_SAMP_FREQ_44,
    I2S_SAMP_FREQ_32,
    I2S_SAMP_FREQ_16
} i2s_samp_freq;

void i2s_init(bool is_mono, const i2s_samp_freq freq);
void i2s_start(void);
void i2s_stop(void);
bool i2s_apll_freq_step(bool is_incr);
#endif