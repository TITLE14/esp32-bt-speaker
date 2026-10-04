#ifndef I2S_DRIVER_H
#define I2S_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

#define DMA_BUF_SIZE   1024
#define DMA_BUF_COUNT  10

typedef enum : uint8_t {
    I2S_SAMP_RATE_48,
    I2S_SAMP_RATE_44
} i2s_samp_freq;

void i2s_init();
void i2s_configure(bool is_mono, const i2s_samp_freq freq);
void i2s_start(void);
void i2s_stop(void);
bool i2s_clock_tune(bool is_incr);
#endif