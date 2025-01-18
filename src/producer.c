// SPDX-License-Identifier: BSD-3-Clause

#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "ring_buffer.h"
#include "packet.h"
#include "utils.h"
#include "producer.h"

void publish_data(so_ring_buffer_t *rb, const char *filename)
{
	char buffer[PKT_SZ];
	ssize_t sz;
	int fd;

	fd = open(filename, O_RDONLY);
	DIE(fd < 0, "open");

	while ((sz = read(fd, buffer, PKT_SZ)) != 0) {
		DIE(sz != PKT_SZ, "packet truncated");

		pthread_mutex_lock(&(rb->mut_buff));
		while (rb->len > rb->cap - PKT_SZ)
			pthread_cond_wait(&(rb->cond_empty), &(rb->mut_buff));
		/* enqueue packet into ring buffer */
		ring_buffer_enqueue(rb, buffer, sz);

		pthread_cond_signal(&(rb->cond_full));
		pthread_mutex_unlock(&(rb->mut_buff));
	}
	ring_buffer_stop(rb);
}
