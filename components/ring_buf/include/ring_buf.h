#ifndef RING_BUF_H
#define RING_BUF_H

#include "freertos/idf_additions.h"
#include "stdint.h"

#define RING_SIZE 65536u
#define RING_MASK (RING_SIZE - 1u)

void ring_notify_on_fill(TaskHandle_t notify_task, uint32_t notify_threshold);
uint32_t ring_free_space(void);
uint32_t ring_used(void);
void ring_zeroing(void);
void ring_push(const uint8_t* source, uint32_t n_bytes_to_push);
void ring_read(uint8_t* dest, uint32_t n_bytes_to_read);

void ring_init(void);
uint64_t get_write_count(void);
uint64_t get_read_count(void);

#endif