#ifndef RING_WATCHER_H
#define RING_WATCHER_H

#include <stdint.h>

void ring_watcher_start(void);
void ring_watcher_stop(void);
void ring_watcher_init(uint8_t deadband_radius_percent, uint16_t sample_count, uint16_t sampling_interval_ms);

#endif