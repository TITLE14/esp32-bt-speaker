#ifndef I2S_DRIVER_H
#define I2S_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum : uint8_t {
    I2S_SAMP_RATE_48,
    I2S_SAMP_RATE_44
} i2s_samp_freq;

/**
 * @brief Initializes the custom I2S transmitter and its DMA resources.
 *
 * Configures I2S0 for audio output and starts the worker tasks required for playback.
 */
void i2s_init();

/**
 * @brief Configures the I2S transmitter for the specified audio format.
 *
 * @param is_mono Whether to configure mono output.
 * @param freq Audio sample frequency.
 */
void i2s_configure(bool is_mono, const i2s_samp_freq freq);

/**
 * @brief Starts I2S audio playback after preparing the DMA buffers.
 *
 * Playback begins after the ring buffer reaches the configured start threshold.
 */
void i2s_start(void);

/**
 * @brief Stops I2S audio transmission.
 */
void i2s_stop(void);

/**
 * @brief Adjusts the I2S source clock by one tuning step.
 *
 * @param is_incr Increase the clock when true, otherwise decrease it.
 * @return true if the clock was adjusted, false if the limit was reached.
 */
bool i2s_clock_tune(bool is_incr);

#endif