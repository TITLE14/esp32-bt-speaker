#include "ring_buf.h"
#include "string.h"
#include <stdatomic.h>
#include <stdint.h>

static uint8_t ring_buf[RING_SIZE];
static atomic_uint write_idx = 0;
static atomic_uint read_idx = 0;

uint32_t ring_used(void) {
    return (atomic_load_explicit(&write_idx, memory_order_acquire) -
        atomic_load_explicit(&read_idx, memory_order_acquire)) & RING_MASK;
}

uint32_t ring_free_space(void) {
    return RING_MASK - ring_used();
}

static void write_idx_incr(uint32_t bytes_to_incr) {
    uint32_t new_idx = (atomic_load_explicit(&write_idx, memory_order_relaxed) + bytes_to_incr) & RING_MASK;
    atomic_store_explicit(&write_idx, new_idx, memory_order_release);
}

void ring_read_idx_incr(uint32_t bytes_to_incr) {
    uint32_t read = atomic_load_explicit(&read_idx, memory_order_relaxed);
    read = (read + bytes_to_incr) & RING_MASK;

    atomic_store_explicit(&read_idx, read, memory_order_release);
}

void ring_zeroing(void) {
    memset(ring_buf, 0, sizeof(ring_buf));
}

void ring_push(const uint8_t* source, uint32_t n_bytes_to_push) {
    uint32_t write = atomic_load_explicit(&write_idx, memory_order_relaxed);
    uint32_t new_idx = write + n_bytes_to_push;

    if (new_idx >= RING_SIZE) {
        uint32_t first_half = RING_SIZE - write;
        memcpy(&ring_buf[write], source, first_half);

        uint32_t second_half = n_bytes_to_push - first_half;
        memcpy(&ring_buf[0], source + first_half, second_half);

        write_idx_incr(n_bytes_to_push);
    } else {
        memcpy(&ring_buf[write], source, n_bytes_to_push);
        write_idx_incr(n_bytes_to_push);
    }
}

void ring_read(uint8_t* dest, uint32_t n_bytes_to_read) {
    uint32_t read = atomic_load_explicit(&read_idx, memory_order_relaxed);
    atomic_load_explicit(&write_idx, memory_order_acquire);

    uint32_t new_idx = read + n_bytes_to_read;
    
    if (new_idx >= RING_SIZE) {
        uint32_t first_half = RING_SIZE - read;
        memcpy(dest, &ring_buf[read], first_half);

        uint32_t second_half = n_bytes_to_read - first_half;
        memcpy(dest + first_half, &ring_buf[0], second_half);
    } else {
        memcpy(dest, &ring_buf[read], n_bytes_to_read);
    }
}