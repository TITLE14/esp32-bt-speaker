#include "ring_buf.h"
#include "string.h"
#include "math.h"

static uint8_t ring_buf[RING_SIZE];
static uint32_t write_idx = 16384;//16384;
static uint32_t read_idx = 0;

uint16_t ring_used(void) {
    return (write_idx - read_idx) & RING_MASK;
}

uint16_t ring_free_space(void) {
    return (RING_SIZE - ring_used()) & RING_MASK;
}

static void write_idx_incr(uint32_t bytes_to_incr) {
    write_idx = (write_idx + bytes_to_incr) & RING_MASK;
}

void ring_read_idx_incr(uint32_t bytes_to_incr) {
    uint32_t buf_used = ring_used();
    uint32_t val_to_incr = fmin(buf_used, bytes_to_incr);

    read_idx = (read_idx + val_to_incr) & RING_MASK;
}

void ring_zeroing(void) {
    memset(ring_buf, 0, sizeof(ring_buf));
}

void ring_push(const uint8_t* source, uint32_t n_bytes_to_push) {
    uint32_t new_idx = write_idx + n_bytes_to_push;

    if (new_idx >= RING_SIZE) {
        uint32_t first_half = RING_SIZE - write_idx;
        memcpy(&ring_buf[write_idx], source, first_half);

        uint32_t second_half = n_bytes_to_push - first_half;
        memcpy(&ring_buf[0], source + first_half, second_half);

        write_idx_incr(n_bytes_to_push);
    } else {
        memcpy(&ring_buf[write_idx], source, n_bytes_to_push);
        write_idx_incr(n_bytes_to_push);
    }
}

void ring_read(uint8_t* dest, uint32_t n_bytes_to_read) {
    uint32_t new_idx = read_idx + n_bytes_to_read;
    
    if (new_idx >= RING_SIZE) {
        uint32_t first_half = RING_SIZE - read_idx;
        memcpy(dest, &ring_buf[read_idx], first_half);

        uint32_t second_half = n_bytes_to_read - first_half;
        memcpy(dest + first_half, &ring_buf[0], second_half);
    } else {
        memcpy(dest, &ring_buf[read_idx], n_bytes_to_read);
    }
}