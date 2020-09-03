/*
 * lottery.c
 *
 *  Created on: Jun 21, 2020
 *      Author: krad2
 */

#include "lottery.h"

/*-----------------------------------------------------------*/

#ifndef max
#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
	   __typeof__ (b) _b = (b); \
	 _a > _b ? _a : _b; })
#endif

#ifndef min
#define min(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })
#endif

/*-----------------------------------------------------------*/

static int lottery_client_cmp(const void *left, const void *right) {
    lottery_client_t *a = rb_entry_safe(left, lottery_client_t, rq_entry);
    lottery_client_t *b = rb_entry_safe(right, lottery_client_t, rq_entry);

    return a->shares - b->shares;
}

static void lottery_client_copy(const void *src, void *dst) {
    lottery_client_t *a = rb_entry_safe(src, lottery_client_t, rq_entry);
    lottery_client_t *b = rb_entry_safe(dst, lottery_client_t, rq_entry);

    b->shares = a->shares;
}

/*-----------------------------------------------------------*/

static void lottery_client_init(lottery_client_t *client, unsigned int priority) {
	client->shares = priority;

	rb_node_init(&client->rq_entry);
}

static void lottery_client_update(lottery_client_t *client, unsigned int priority) {
	client->shares = priority;
}

/*-----------------------------------------------------------*/

static void lottery_client_add_to_list(lottery_mgr_t *mgr, lottery_client_t *client) {

}

static void lottery_client_remove_from_list(lottery_mgr_t *mgr, lottery_client_t *client) {

}

/*-----------------------------------------------------------*/

static void lottery_mgr_init(lottery_mgr_t *mgr) {
	rb_tree_rcached_init(&mgr->rq);

}

static void lottery_mgr_start(lottery_mgr_t *mgr) {

}

static void lottery_mgr_end(lottery_mgr_t *mgr) {
	rb_tree_clean(&mgr->rq.tree);
}

static int lottery_mgr_run(lottery_mgr_t *mgr) {

}

static void lottery_mgr_yield(lottery_mgr_t *mgr) {
	lottery_mgr_run(mgr);
}

// needs to perform a random query for the tree defined by all of the higher priority threads
static void lottery_mgr_yield_higher(lottery_mgr_t *mgr) {

}

static void lottery_mgr_sleep(lottery_mgr_t *mgr, lottery_mgr_t *client) {
	lottery_client_remove_from_list(mgr, client);	// simply remove the entry from the list
}

static void lottery_mgr_wakeup(lottery_mgr_t *mgr, lottery_client_t *client) {
	lottery_client_add_to_list(mgr, client);
}

/*-----------------------------------------------------------*/

void lottery_init(lottery_mgr_t *sched) {
	lottery_mgr_init(sched);
}

void lottery_start(lottery_mgr_t *sched) {
	lottery_mgr_start(sched);
}

void lottery_add(lottery_mgr_t *sched, lottery_client_t *client, unsigned int priority) {
	lottery_client_init(client, priority);
	lottery_client_add_to_list(sched, client);
}

void lottery_register(lottery_mgr_t *sched, lottery_client_t *client) {
	lottery_client_add_to_list(sched, client);
}

void lottery_deregister(lottery_mgr_t *sched, lottery_client_t *client) {
	lottery_mgr_sleep(sched, client);
}

void lottery_reregister(lottery_mgr_t *sched, lottery_client_t *client, unsigned int priority) {
	lottery_deregister(sched, client);
	lottery_client_update(client, priority);
	lottery_register(sched, client);
}

void lottery_delete(lottery_mgr_t *sched, lottery_client_t *client) {
	lottery_deregister(sched, client);
	lottery_client_reset(client);
}

void lottery_end(lottery_mgr_t *sched) {
	lottery_mgr_end(sched);
}

void lottery_run(lottery_mgr_t *sched) {
	lottery_mgr_run(sched);
}

void lottery_yield(lottery_mgr_t *sched) {
	lottery_mgr_yield(sched);
}

void lottery_yield_higher(lottery_mgr_t *sched) {
	lottery_mgr_yield_higher(sched);
}
