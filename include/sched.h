/*
 * sched.h
 *
 *  Created on: Jun 2, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_SCHED_H_
#define INCLUDE_SCHED_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "port_config.h"

#include "irq.h"
#include "thread.h"
#include "sleep_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct thread thread_t;

typedef unsigned int sched_status_t;

//typedef struct sched {
//	sched_impl_mgr_t instance;
//	sleep_queue_t sleep_mgr;
//
//	sched_status_t state;
//	thread_t *sched_active_thread;
//
//	#ifdef CONFIG_USE_KERNEL_STACK
//		uint8_t sched_isr_stack[CONFIG_ISR_STACK_SIZE];
//	#endif
//
//	#ifdef CONFIG_USE_TICK_HOOK
//		void (*sched_tick_cb)(void *arg);
//		void *tick_hook_params;
//	#endif
//
//} sched_t;
//
//#define SCHED_STATUS_IN_IRQ_POS 								(0)
//#define SCHED_STATUS_CONTEXT_SWITCH_REQUEST_POS 				(1)
//#define SCHED_STATUS_IRQ_LOCKED_POS 							(2)
//#define SCHED_STATUS_THREAD_COUNT_POS							(3)
//
//#define SCHED_STATUS_IN_IRQ_MASK 								(1 << SCHED_STATUS_IN_IRQ_POS)
//#define SCHED_STATUS_CONTEXT_SWITCH_REQUEST_MASK 				(1 << SCHED_STATUS_CONTEXT_SWITCH_REQUEST_POS)
//#define SCHED_STATUS_IRQ_LOCKED_MASK 							(1 << SCHED_STATUS_IRQ_LOCKED_POS)
//
//#define SCHED_STATUS_THREAD_COUNT_MASK 							~(SCHED_STATUS_IN_IRQ_MASK | \
//																SCHED_STATUS_CONTEXT_SWITCH_REQUEST_MASK | \
																SCHED_STATUS_IRQ_LOCKED_MASK)

//#define SCHED_STATUS_IN_IRQ										SCHED_STATUS_IN_IRQ_MASK
//#define SCHED_STATUS_CONTEXT_SWITCH_REQUEST 					SCHED_STATUS_CONTEXT_SWITCH_REQUEST_MASK
//#define SCHED_STATUS_IRQ_LOCKED									SCHED_STATUS_IRQ_LOCKED_MASK

//extern volatile sched_t sched_g;
//#define sched_impl_active_client(mptr)	container_of(__sched_impl_active_client(mptr), thread_t, cli)

typedef enum {
    STATUS_STOPPED,
    STATUS_ZOMBIE,
    STATUS_SLEEPING,
    STATUS_MUTEX_BLOCKED,
    STATUS_RECEIVE_BLOCKED,
    STATUS_SEND_BLOCKED,
    STATUS_REPLY_BLOCKED,
    STATUS_FLAG_BLOCKED_ANY,
    STATUS_FLAG_BLOCKED_ALL,
    STATUS_MBOX_BLOCKED,
    STATUS_COND_BLOCKED,
    STATUS_RUNNING,
    STATUS_PENDING,
    STATUS_NUMOF
} thread_status_t;

void sched_init(void);

void sched_add(volatile thread_t *new, volatile unsigned int priority);

void sched_register(volatile thread_t *new);

void sched_deregister(volatile thread_t *new);

void sched_reregister(volatile thread_t *new, volatile unsigned int priority);

void sched_start(void);

void sched_end(void);

void sched_run(void);

void sched_yield(void);

void sched_yield_higher(void);

sched_status_t sched_get_status(void);

void sched_set_status(sched_status_t status);

void sched_clear_status(sched_status_t status);

thread_t *sched_current_thread(void);
unsigned int sched_thread_count(void);

void sched_register_cb(void (*cb)(void *arg), void *params);
void sched_task_exit(void);
#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_SCHED_H_ */
