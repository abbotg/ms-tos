/*
 * sched_impl.h
 *
 *  Created on: Jun 4, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_SCHEDULERS_SCHED_IMPL_H_
#define INCLUDE_SCHEDULERS_SCHED_IMPL_H_

#include "port_config.h"

#include "sleep_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------*/

#define DECLARE_SCHED_IMPL(type)											\
	typedef type##_mgr_t sched_impl_mgr_t;									\
	typedef type##_client_t sched_impl_client_t;							\

/*-----------------------------------------------------------*/

#if (CONFIG_SCHED_RR == 1)
	#define SCHED_ALG_PATH 							"rr/rr.h"
#elif (CONFIG_SCHED_VTRR == 1)
	#define SCHED_ALG_PATH 							"vtrr/vtrr.h"
#elif (CONFIG_SCHED_LOTTERY == 1)
	#define SCHED_ALG_PATH 							"lottery/lottery.h"
#elif (CONFIG_SCHED_MULTIQ == 1)
	#define SCHED_ALG_PATH							"multiq/multiq.h"
#else
	#error "No scheduling algorithm configured by CONFIG_SCHED_*"
#endif

/*-----------------------------------------------------------*/

#include SCHED_ALG_PATH
DECLARE_SCHED_IMPL(vtrr);

typedef struct thread_impl thread_impl_t;
typedef unsigned int sched_status_t;

#define SCHED_STATUS_IN_IRQ_POS 								(13)
#define SCHED_STATUS_CONTEXT_SWITCH_REQUEST_POS 				(14)
#define SCHED_STATUS_IRQ_LOCKED_POS 							(15)
#define SCHED_STATUS_THREAD_COUNT_POS							(0)

#define SCHED_STATUS_IN_IRQ_MASK 								(1 << SCHED_STATUS_IN_IRQ_POS)
#define SCHED_STATUS_CONTEXT_SWITCH_REQUEST_MASK 				(1 << SCHED_STATUS_CONTEXT_SWITCH_REQUEST_POS)
#define SCHED_STATUS_IRQ_LOCKED_MASK 							(1 << SCHED_STATUS_IRQ_LOCKED_POS)

#define SCHED_STATUS_THREAD_COUNT_MASK 							~(SCHED_STATUS_IN_IRQ_MASK | \
																SCHED_STATUS_CONTEXT_SWITCH_REQUEST_MASK | \
																SCHED_STATUS_IRQ_LOCKED_MASK)

#define SCHED_STATUS_IN_IRQ										SCHED_STATUS_IN_IRQ_MASK
#define SCHED_STATUS_CONTEXT_SWITCH_REQUEST 					SCHED_STATUS_CONTEXT_SWITCH_REQUEST_MASK
#define SCHED_STATUS_IRQ_LOCKED									SCHED_STATUS_IRQ_LOCKED_MASK

typedef struct sched_impl {
	sched_impl_mgr_t instance;
	sleep_queue_t sleep_mgr;

	sched_status_t state;
	thread_impl_t *sched_active_thread;

	void *boot_context;

	#ifdef CONFIG_USE_KERNEL_STACK
		uint8_t sched_isr_stack[CONFIG_ISR_STACK_SIZE];
	#endif

	#ifdef CONFIG_USE_TICK_HOOK
		void (*sched_tick_cb)(void *arg);
		void *tick_hook_params;
	#endif
} sched_impl_t;

extern volatile sched_impl_t sched_p;

void sched_impl_init(void);
void sched_impl_add(thread_impl_t *client, unsigned int priority);
void sched_impl_register(thread_impl_t *client);
void sched_impl_deregister(thread_impl_t *client);
void sched_impl_reregister(thread_impl_t *client, unsigned int priority);
void sched_impl_start(void);
void sched_impl_end(void);
void sched_impl_run(void);
void sched_impl_yield(void);
void sched_impl_yield_higher(void);

#ifdef __cplusplus
}
#endif


#endif /* INCLUDE_SCHEDULERS_SCHED_IMPL_H_ */
