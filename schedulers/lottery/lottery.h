/*
 * lottery.h
 *
 *  Created on: Jun 4, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_SCHEDULERS_LOTTERY_H_
#define INCLUDE_SCHEDULERS_LOTTERY_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "rbtree.h"

typedef struct sched_lottery_client {
	unsigned int shares;
	rbnode rq_entry;
} lottery_client_t;

typedef struct sched_lottery_mgr {
	rbtree_rcached rq;
} lottery_mgr_t;

void lottery_init(lottery_mgr_t *sched);
void lottery_start(lottery_mgr_t *sched);
void lottery_add(lottery_mgr_t *sched, lottery_client_t *client, unsigned int priority);
void lottery_register(lottery_mgr_t *sched, lottery_client_t *client);
void lottery_deregister(lottery_mgr_t *sched, lottery_client_t *client);
void lottery_reregister(lottery_mgr_t *sched, lottery_client_t *client, unsigned int priority);
void lottery_delete(lottery_mgr_t *sched, lottery_client_t *client);
void lottery_end(lottery_mgr_t *sched);
void lottery_run(lottery_mgr_t *sched);
void lottery_yield(lottery_mgr_t *sched);
void lottery_yield_higher(lottery_mgr_t *sched);

#endif /* INCLUDE_SCHEDULERS_LOTTERY_H_ */
