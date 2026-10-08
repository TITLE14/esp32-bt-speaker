#ifndef RING_BUF_H
#define RING_BUF_H

#include "freertos/idf_additions.h"
#include "stdint.h"

#define RING_SIZE 65536u
#define RING_MASK (RING_SIZE - 1u)

/**
 * @brief Initializes the ring buffer
 */
void ring_init(void);
/**
 * @brief Notifies a task when the ring buffer reaches the specified fill level.
 *
 * The notification is triggered once per registration.
 *
 * @param notify_task Task to notify.
 * @param notify_threshold Buffer fill level that triggers the notification.
 */
void ring_notify_on_fill(TaskHandle_t notify_task, uint32_t notify_threshold);

/**
 * @brief Returns the number of bytes currently stored in the ring buffer.
 */
uint32_t ring_free_space(void);

/**
 * @brief Returns the remaining free space in the ring buffer.
 */
uint32_t ring_used(void);

/**
 * @brief Appends data to the ring buffer.
 *
 * @param source Source data.
 * @param n_bytes_to_push Number of bytes to append.
 */
void ring_push(const uint8_t* source, uint32_t n_bytes_to_push);

/**
 * @brief Reads data from the ring buffer.
 *
 * @param dest Destination buffer.
 * @param n_bytes_to_read Number of bytes to read.
 */
void ring_read(uint8_t* dest, uint32_t n_bytes_to_read);

#endif