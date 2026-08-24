#ifndef RING_BUF_H
#define RING_BUF_H

#include "stdint.h"

#define RING_SIZE 32768u
#define RING_MASK (RING_SIZE - 1u)

uint32_t ring_free_space(void);
uint32_t ring_used(void);
void ring_zeroing(void);
void ring_push(const uint8_t* source, uint32_t n_bytes_to_push);
//read index needs to increased manually!
void ring_read(uint8_t* dest, uint32_t n_bytes_to_read);
void ring_read_idx_incr(uint32_t bytes_to_incr);

#endif