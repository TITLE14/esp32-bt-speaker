#ifndef RING_BUF_H
#define RING_BUF_H

#include "stdint.h"

#define RING_SIZE 16384u
#define RING_MASK (RING_SIZE - 1u)

void zeroing(void);
void ring_push(int16_t* buf, uint16_t n_bytes_to_push);
void ring_read(uint16_t n_bytes_to_read, int16_t* dest);
void write_idx_incr(uint16_t samps_to_incr);
void read_idx_incr(uint16_t samps_to_incr);
inline uint16_t get_write_idx(void);
inline uint16_t get_read_idx(void);

#endif