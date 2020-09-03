/*
 * thread_impl.h
 *
 *  Created on: Jun 26, 2020
 *      Author: krad2
 */

#ifndef PRIVATE_THREAD_IMPL_H_
#define PRIVATE_THREAD_IMPL_H_

#include "sleep_queue.h"
#include "sched_impl.h"

typedef struct thread_impl {
	void *sp;
	sched_impl_client_t rq_entry;
	sleep_queue_entry_t sq_entry;
} thread_impl_t;

typedef int (*thread_fn_t)(void *);

void thread_impl_init(thread_impl_t *me, void *sp, thread_fn_t runnable, void *args);

#endif /* PRIVATE_THREAD_IMPL_H_ */
