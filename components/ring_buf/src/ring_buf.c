#include "ring_buf.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "string.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "app_logger.h"

static uint8_t *ring_buf = NULL;
static atomic_uint write_idx = 0;
static atomic_uint read_idx = 0;

static EventGroupHandle_t evt_group = NULL;
static EventBits_t evt_mask = 0;
static uint32_t ntf_threshold = 0;
static atomic_bool notify_enabled = false;

static void write_idx_incr(uint32_t bytes_to_incr) {
    if (ring_free_space() < bytes_to_incr) {
        APP_LOGW("RING", "No free space, incr skipped!");
        return;
    }
    uint32_t new_idx = (atomic_load_explicit(&write_idx, memory_order_relaxed) + bytes_to_incr) & RING_MASK;
    atomic_store_explicit(&write_idx, new_idx, memory_order_release);
}

static void read_idx_incr(uint32_t bytes_to_incr) {
    if(ring_used() < bytes_to_incr) {
        return;
    }
    uint32_t read = atomic_load_explicit(&read_idx, memory_order_relaxed);
    read = (read + bytes_to_incr) & RING_MASK;

    atomic_store_explicit(&read_idx, read, memory_order_release);
}

void ring_init(void) {
    ring_buf = (uint8_t*)malloc(RING_SIZE);
    if (ring_buf == NULL) {
        ESP_LOGE("RING", "ring_buf malloc failed");
        abort();
    }
}

void ring_notify_on_fill(EventGroupHandle_t event_group, EventBits_t event_mask, uint32_t notify_threshold) {
    evt_group = event_group;
    ntf_threshold = notify_threshold;
    evt_mask = event_mask;
    atomic_exchange_explicit(&notify_enabled, true, memory_order_acq_rel);

    if (ring_used() >= notify_threshold && atomic_exchange_explicit(&notify_enabled, false, memory_order_acq_rel)) {
        xEventGroupSetBits(evt_group, evt_mask);
        return;
    }
}

uint32_t ring_used(void) {
    return (atomic_load_explicit(&write_idx, memory_order_acquire) -
        atomic_load_explicit(&read_idx, memory_order_acquire)) & RING_MASK;
}

uint32_t ring_free_space(void) {
    return RING_MASK - ring_used();
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

    if (atomic_load_explicit(&notify_enabled, memory_order_acquire) && ring_used() >= ntf_threshold &&
    atomic_exchange_explicit(&notify_enabled, false, memory_order_acq_rel)) {
        xEventGroupSetBits(evt_group, evt_mask);
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

        read_idx_incr(n_bytes_to_read);
    } else {
        memcpy(dest, &ring_buf[read], n_bytes_to_read);
        read_idx_incr(n_bytes_to_read);
    }
}