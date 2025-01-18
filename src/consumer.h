/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef _SO_CONSUMER_H_
#define _SO_CONSUMER_H_

#include <stdio.h>
#include <pthread.h>
#include "ring_buffer.h"
#include "packet.h"

typedef struct so_consumer_ctx_t {
	struct so_ring_buffer_t *producer_rb;
	int num_consumers;
	int out_fd;

    /* TODO: add synchronization primitives for timestamp ordering */
	struct log **arr;
	int id;
} so_consumer_ctx_t;

typedef struct log {
	int action;
	unsigned long hash;
	unsigned long timestamp;
} log;

int create_consumers(pthread_t *tids,
					int num_consumers,
					so_ring_buffer_t *rb,
					const char *out_filename);

#endif /* _SO_CONSUMER_H_ */
