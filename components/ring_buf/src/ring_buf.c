#include "ring_buf.h"
#include "string.h"
#include "math.h"

static int16_t ring_buf[RING_SIZE];
static uint16_t write_idx = 5000;//8192;
static uint16_t read_idx = 0;

uint16_t ring_used(void) {
    return (write_idx - read_idx) & RING_MASK;
}

uint16_t ring_space(void) {
    return (RING_SIZE - ring_used()) & RING_MASK;
}

void write_idx_incr(uint16_t samps_to_incr) {
    write_idx = (write_idx + samps_to_incr) & RING_MASK;
}

void read_idx_incr(uint16_t samps_to_incr) {
    uint16_t buf_used = ring_used();
    uint16_t val_to_incr = fmin(buf_used, samps_to_incr);

    read_idx = (read_idx + val_to_incr) & RING_MASK;
}

uint16_t get_read_idx(void) {
    return read_idx;
}

uint16_t get_write_idx(void) {
    return write_idx;
}

void zeroing(void) {
    memset(ring_buf, 0, sizeof(ring_buf));
}

void ring_push(int16_t* buf, uint16_t n_bytes_to_push) {
    uint16_t new_idx = write_idx + (n_bytes_to_push / 2);

    if (new_idx > RING_SIZE) {
        uint16_t first_half = (RING_SIZE - write_idx) * 2;
        memcpy(&ring_buf[write_idx], buf, first_half);

        uint16_t second_half = n_bytes_to_push - first_half;
        memcpy(&ring_buf[0], (buf + (first_half / 2)), second_half);

        write_idx_incr(n_bytes_to_push / 2);
    } else {
        memcpy(&ring_buf[write_idx], buf, n_bytes_to_push);
        write_idx_incr(n_bytes_to_push / 2);
    }
}

void ring_read(uint16_t n_bytes_to_read, int16_t* dest) {
    uint16_t samps = n_bytes_to_read / 2;

    uint16_t new_idx = read_idx + samps;
    
    if (new_idx > RING_SIZE) {
        uint16_t first_half = (RING_SIZE - read_idx) * 2;
        memcpy(dest, &ring_buf[read_idx], first_half);

        uint16_t second_half = n_bytes_to_read - first_half;
        memcpy(dest + (first_half / 2), &ring_buf[0], second_half);
    } else {
        memcpy(dest, &ring_buf[read_idx], n_bytes_to_read);
    }
}