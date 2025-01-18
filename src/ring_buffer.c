// SPDX-License-Identifier: BSD-3-Clause

#include "ring_buffer.h"
#include "utils.h"
#include "packet.h"

int ring_buffer_init(so_ring_buffer_t *ring, size_t cap)
{
	/* TODO: implement ring_buffer_init */
	// ring = malloc(sizeof(so_ring_buffer_t));
	// DIE(ring == NULL, "malloc failed");

	ring->cap = cap;
	ring->len = 0;
	ring->read_pos = 0;
	ring->write_pos = 0;
	ring->exit = 0;

	ring->data = malloc(cap);
	DIE(ring->data == NULL, "malloc failed");
	pthread_cond_init(&(ring->cond_empty), NULL);
	pthread_cond_init(&(ring->cond_full), NULL);
	pthread_mutex_init(&(ring->mut_buff), NULL);
	return 1;
}

ssize_t ring_buffer_enqueue(so_ring_buffer_t *ring, void *data, size_t size)
{
	/* TODO: implement ring_buffer_enqueue */
	if (ring->len > ring->cap - size)
		return -1;
	char *d = (char *)data;
	char *rd = ring->data;

	for (size_t i = 0; i < size; i++) {
		rd[ring->write_pos] = d[i];
		ring->write_pos = (ring->write_pos + 1) % ring->cap;
	}
	ring->len += size;
	return ring->len;
}

ssize_t ring_buffer_dequeue(so_ring_buffer_t *ring, void *data, size_t size)
{
	/* TODO: Implement ring_buffer_dequeue */
	if (ring->len < size)
		return -1;
	char *d = (char *)data;
	char *rd = ring->data;

	for (size_t i = 0; i < size; i++) {
		d[i] = rd[ring->read_pos];
		ring->read_pos = (ring->read_pos + 1) % ring->cap;
	}
	ring->len -= size;
	return ring->len;
}

void ring_buffer_destroy(so_ring_buffer_t *ring)
{
	/* TODO: Implement ring_buffer_destroy */
	pthread_cond_destroy(&(ring->cond_empty));
	pthread_cond_destroy(&(ring->cond_full));
	pthread_mutex_destroy(&(ring->mut_buff));
}

void ring_buffer_stop(so_ring_buffer_t *ring)
{
	/* TODO: Implement ring_buffer_stop */
	pthread_mutex_lock(&(ring->mut_buff));
	ring->exit = 1;
	pthread_cond_broadcast(&(ring->cond_full));
	pthread_mutex_unlock(&(ring->mut_buff));
}
