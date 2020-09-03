/*
 * vtrr.h
 *
 *  Created on: Jun 4, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_SCHEDULERS_VTRR_H_
#define INCLUDE_SCHEDULERS_VTRR_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>

#include "rbtree.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------*/

/**
 * @name Virtual-time round-robin scheduling management structures.
 * @{
 */

typedef struct sched_vtrr_client {
	unsigned int shares;		/* thread priority */
	unsigned int runs_left;		/* number of quanta left to go */
	unsigned int fin_time;		/* virtual timestamp for VTRR allocation computations */
	unsigned int timestep;		/* virtual progress amount for each timestep */

	rbnode rq_entry;			/* red-black tree entry for sorted order */
} vtrr_client_t;

typedef struct sched_vtrr_mgr {
	unsigned int shares;		/* total number of slices to hand out per cycle */
	unsigned int runs_left;		/* number of slices remaining before a new scheduling cycle */
	unsigned int group_time;	/* virtual timestamp for thread progress comparisons */
	unsigned int timestep;		/* virtual progress amount for each timestep */

	rbtree_rcached rq;			/* red-black tree for sorted threads, maximum is cached */
	rbnode *curr_max;			/* pointer to the highest priority runnable thread */

	rbnode *curr_cli;			/* pointer to the currently running thread */
	rbnode *next_cli;			/* pointer to the thread scheduled for the next timeslice */
} vtrr_mgr_t;

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name Simple container_of() semantic macros provided to access threads through their bookkeeping.
 * @{
 */

#define __vtrr_entry(ptr) rb_entry((ptr), vtrr_client_t, rq_entry)
#define vtrr_entry(ptr) __vtrr_entry((ptr))
#define vtrr_active_client(mptr) __vtrr_entry((mptr)->curr_cli)
#define vtrr_next_client(mptr) __vtrr_entry((mptr)->next_cli)

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name VTRR wrapper functions.
 * @{
 */

/**
 * @brief Initializes a blank VTRR manager for thread installation.
 * @param[in] sched	Pointer to a vtrr_mgr_t instance.
 */
void vtrr_init(vtrr_mgr_t *sched);

/**
 * @brief Adds a new thread to the VTRR run queue.
 * @param[in] sched	Pointer to a vtrr_mgr_t instance.
 * @param[in] priority Thread scheduling priority.
 */
void vtrr_add(vtrr_mgr_t *sched, vtrr_client_t *client, unsigned int priority);

/**
 * @brief Adds an already-initialized thread to the run queue.
 * @details does not allocate a thread.
 * @param[in] sched	Pointer to a vtrr_mgr_t instance.
 * @param[in] client Thread to be added.
 */
void vtrr_register(vtrr_mgr_t *sched, vtrr_client_t *client);

/**
 * @brief Removes an already-initialized thread from the run queue.
 * @details does not deallocate thread.
 * @param[in] sched	Pointer to a vtrr_mgr_t instance.
 * @param[in] client Thread to be removed.
 */
void vtrr_deregister(vtrr_mgr_t *sched, vtrr_client_t *client);

/**
 * @brief Updates thread position in the run queue.
 * @details does not deallocate thread.
 * @param[in] sched	Pointer to a vtrr_mgr_t instance.
 * @param[in] client Thread to be updated.
 * @param[in] priority New thread priority.
 */
void vtrr_reregister(vtrr_mgr_t *sched, vtrr_client_t *client, unsigned int priority);

/**
 * @brief Readies the VTRR manager for timeslicing.
 * @details At least 1 thread must be installed for the manager to start.
 */
void vtrr_start(vtrr_mgr_t *sched);

/**
 * @brief Kills the VTRR manager and cleans the run queue.
 */
void vtrr_end(vtrr_mgr_t *sched);

/**
 * @brief Schedules the next thread in VTRR-logical order.
 */
void vtrr_run(vtrr_mgr_t *sched);

/**
 * @brief Invokes vtrr_run() to change the active thread in VTRR-logical order.
 */
void vtrr_yield(vtrr_mgr_t *sched);

/**
 * @brief Changes the active thread to the highest priority runnable thread if it is higher priority than the active thread.
 */
void vtrr_yield_higher(vtrr_mgr_t *sched);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_SCHEDULERS_VTRR_H_ */
