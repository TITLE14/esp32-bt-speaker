#ifndef RING_WATCHER_H
#define RING_WATCHER_H

#include <stdint.h>

/**
 * @brief Starts ring buffer level monitoring.
 */
void ring_watcher_start(void);

/**
 * @brief Stops ring buffer level monitoring.
 */
void ring_watcher_stop(void);

/**
 * @brief Initializes ring buffer level monitoring.
 *
 * @param deadband_radius_percent Half-width of the target fill deadband as a percentage of the buffer size.
 * @param sample_count Number of samples used for monitoring.
 * @param sampling_interval_ms Interval between samples in milliseconds.
 */
void ring_watcher_init(uint8_t deadband_radius_percent, uint16_t sample_count, uint16_t sampling_interval_ms);

#endif