/*
 * sleep_queue.c
 *
 *  Created on: Jun 24, 2020
 *      Author: krad2
 */

#include "rtos.h"
#include "sleep_queue.h"

void sleep_queue_init(sleep_queue_t *que) {
	rbtree_lcached_init(&que->q);
}

static int sleepq_entry_cmp(const void *left, const void *right) {
	sleep_queue_entry_t *a = rb_entry_safe(left, sleep_queue_entry_t, node);
	sleep_queue_entry_t *b = rb_entry_safe(right, sleep_queue_entry_t, node);

	return (long) a->wake_time - (long) b->wake_time;
}

static void sleepq_entry_copy(const void *src, void *dst) {
	sleep_queue_entry_t *a = rb_entry_safe(src, sleep_queue_entry_t, node);
	sleep_queue_entry_t *b = rb_entry_safe(dst, sleep_queue_entry_t, node);
	b->wake_time = a->wake_time;
}

void sleep_queue_push(sleep_queue_t *que, thread_impl_t *thr, unsigned int wake_time) {
	thr->sq_entry.wake_time = wake_time;
	rb_lcached_insert(&que->q, &thr->sq_entry.node, sleepq_entry_cmp);
}

thread_impl_t *sleep_queue_peek(sleep_queue_t *que) {
	rbnode *ent_node = rb_first_cached(&que->q);
	if (ent_node == 0) return 0;

	sleep_queue_entry_t *ent = rb_entry_safe(ent_node, sleep_queue_entry_t, node);
	return container_of(ent, thread_impl_t, sq_entry);
}

void sleep_queue_pop(sleep_queue_t *que) {
	rb_lcached_delete(&que->q, rb_first_cached(&que->q), sleepq_entry_cmp, sleepq_entry_copy);
}

void sleep_queue_remove_node(sleep_queue_t *que, thread_impl_t *thr) {
	rb_lcached_delete(&que->q, &thr->sq_entry.node, sleepq_entry_cmp, sleepq_entry_copy);
}
