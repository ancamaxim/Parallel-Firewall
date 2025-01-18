// SPDX-License-Identifier: BSD-3-Clause

#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "consumer.h"
#include "ring_buffer.h"
#include "packet.h"
#include "utils.h"

pthread_barrier_t barrier;
pthread_barrier_t barrier1;
int done;

void merge_sort(void *arr, int l, int r)
{
	if (l >= r)
		return;
	so_consumer_ctx_t *ctx = (so_consumer_ctx_t *) arr;
	int m = l + (r - l) / 2;
	int i = l, j = m + 1, k = l;
	struct log *buff[ctx->num_consumers];

	merge_sort(arr, l, m);
	merge_sort(arr, m + 1, r);

	while (i <= m && j <= r) {
		if (ctx->arr[i]->timestamp < ctx->arr[j]->timestamp)
			buff[k++] = ctx->arr[i++];
		else
			buff[k++] = ctx->arr[j++];
	}
	while (i <= m)
		buff[k++] = ctx->arr[i++];
	while (j <= r)
		buff[k++] = ctx->arr[j++];
	for (i = l; i < k; i++)
		ctx->arr[i] = buff[i];
}

void *consumer_thread(void *args)
{
	/* TODO: implement consumer thread */
	so_consumer_ctx_t *ctx = (so_consumer_ctx_t *) args;
	char out_buf[PKT_SZ];
	int len, res;

	while (1) {
		pthread_mutex_lock(&(ctx->producer_rb->mut_buff));
		while (ctx->producer_rb->len < PKT_SZ && ctx->producer_rb->exit == 0)
			pthread_cond_wait(&(ctx->producer_rb->cond_full), &(ctx->producer_rb->mut_buff));

		res = ring_buffer_dequeue(ctx->producer_rb, out_buf, PKT_SZ);

		pthread_cond_signal(&(ctx->producer_rb->cond_empty));
		pthread_mutex_unlock(&(ctx->producer_rb->mut_buff));

		if (res != -1) {
			struct so_packet_t *pkt = (struct so_packet_t *)out_buf;

			ctx->arr[ctx->id]->action = process_packet(pkt);
			ctx->arr[ctx->id]->hash = packet_hash(pkt);
			ctx->arr[ctx->id]->timestamp = pkt->hdr.timestamp;
		} else {
			ctx->arr[ctx->id]->action = -1;
			ctx->arr[ctx->id]->timestamp = __INT_MAX__;
		}

		int res1 = pthread_barrier_wait(&barrier);

		if (res1 == PTHREAD_BARRIER_SERIAL_THREAD) {
			merge_sort(ctx, 0, ctx->num_consumers - 1);
			for (int i = 0; i < ctx->num_consumers; i++) {
				if (ctx->arr[i]->action == -1)
					continue;
				len = snprintf(out_buf, 256, "%s %016lx %lu\n",
					RES_TO_STR(ctx->arr[i]->action), ctx->arr[i]->hash,
					ctx->arr[i]->timestamp);
				write(ctx->out_fd, out_buf, len);
			}
		}
		if (res == -1)
			done = 1;
		pthread_barrier_wait(&barrier1);
		if (done == 1)
			break;
	}
	return NULL;
}

int create_consumers(pthread_t *tids,
					 int num_consumers,
					 struct so_ring_buffer_t *rb,
					 const char *out_filename)
{
	int out_fd;
	so_consumer_ctx_t *ctx, *ctxi;

	out_fd = open(out_filename, O_RDWR|O_CREAT|O_TRUNC, 0666);
	DIE(out_fd < 0, "open");

	ctx = malloc(sizeof(so_consumer_ctx_t));
	ctx->producer_rb = rb;
	ctx->num_consumers = num_consumers;
	ctx->out_fd = out_fd;
	ctx->arr = malloc(sizeof(struct log *) * num_consumers);
	for (int i = 0; i < num_consumers; i++)
		ctx->arr[i] = malloc(sizeof(struct log));

	pthread_barrier_init(&barrier, NULL, num_consumers);
	pthread_barrier_init(&barrier1, NULL, num_consumers);

	for (int i = 0; i < num_consumers; i++) {
		/*
		 * TODO: Launch consumer threads
		 **/
		ctxi = malloc(sizeof(so_consumer_ctx_t));
		memcpy(ctxi, ctx, sizeof(so_consumer_ctx_t));
		ctxi->id = i;
		pthread_create(&tids[i], NULL, &consumer_thread, ctxi);
	}

	return num_consumers;
}
