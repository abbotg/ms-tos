/*
 * sched_impl.c
 *
 *  Created on: Jun 26, 2020
 *      Author: krad2
 */

#include "sched_impl.h"
#include "thread_impl.h"
#include "hal.h"

/*-----------------------------------------------------------*/

#define __sched_impl_active_client_cast(type, mptr) type##_active_client(mptr)

#if (CONFIG_SCHED_RR == 1)
	#define __sched_impl_active_client(mptr)	__sched_impl_active_client_cast(rr, mptr)
#elif (CONFIG_SCHED_VTRR == 1)
	#define __sched_impl_active_client(mptr)	__sched_impl_active_client_cast(vtrr, mptr)
#elif (CONFIG_SCHED_LOTTERY == 1)
	#define __sched_impl_active_client(mptr)	__sched_impl_active_client_cast(lottery, mptr)
#elif (CONFIG_SCHED_MULTIQ == 1)
	#define __sched_impl_active_client(mptr)	__sched_impl_active_client_cast(multiq, mptr)
#else
	#error "No scheduling algorithm configured by CONFIG_SCHED_*"
#endif

#define sched_impl_active_client(mptr)	container_of(__sched_impl_active_client(mptr), thread_impl_t, rq_entry)

/*-----------------------------------------------------------*/

static volatile thread_impl_t sched_idle_thread;
volatile uint8_t idle_stack[CONFIG_IDLE_STACK_SIZE];

static int idle(void *arg) {
	while (1) {
		arch_idle();
	}

	while (1) {
		panic(PANIC_EXPECT_FAIL, "Idle thread unexpectedly returned");
	}

	return 1;
}

volatile sched_impl_t sched_p;

#define DECLARE_SCHED_IMPL_FNS(type)																		\
	void sched_impl_init(void) {																			\
		type##_init((type##_mgr_t *) &sched_p.instance);													\
		sleep_queue_init((sleep_queue_t *) &sched_p.sleep_mgr);												\
		sched_p.state = 0;																					\
		sched_p.sched_active_thread = (thread_impl_t *) &sched_idle_thread;									\
		thread_impl_init((thread_impl_t *) &sched_idle_thread, 												\
						(void *) (idle_stack + CONFIG_IDLE_STACK_SIZE), idle, NULL);						\
	}																										\
																											\
	void sched_impl_add(thread_impl_t *client, unsigned int priority) { 									\
		type##_add((type##_mgr_t *) &sched_p.instance, (type##_client_t *) &client->rq_entry, priority);	\
		sched_p.state += (1 << SCHED_STATUS_THREAD_COUNT_POS);												\
	}																										\
																											\
	void sched_impl_register(thread_impl_t *client) {														\
		type##_register((type##_mgr_t *) &sched_p.instance, &client->rq_entry);								\
		sched_p.state += (1 << SCHED_STATUS_THREAD_COUNT_POS);												\
	}																										\
																											\
	void sched_impl_deregister(thread_impl_t *client) {														\
		type##_deregister((type##_mgr_t *) &sched_p.instance, &client->rq_entry);							\
		sched_p.state -= (1 << SCHED_STATUS_THREAD_COUNT_POS);												\
	}																										\
																											\
	void sched_impl_reregister(thread_impl_t *client, unsigned int priority) {								\
		type##_reregister((type##_mgr_t *) &sched_p.instance, &client->rq_entry, priority);  				\
	}																										\
																											\
	void sched_impl_start(void) {																			\
		type##_start((type##_mgr_t *) &sched_p.instance);													\
	}																										\
																											\
	void sched_impl_end(void) {																				\
		type##_end((type##_mgr_t *) &sched_p.instance);														\
	}																										\
																											\
	void sched_impl_run(void) {																				\
		if ((sched_p.state & SCHED_STATUS_THREAD_COUNT_MASK) >= 1) {										\
			type##_run((sched_impl_mgr_t *) (type##_mgr_t *) &sched_p.instance);							\
			sched_p.sched_active_thread = sched_impl_active_client((type##_mgr_t *) &sched_p.instance);		\
		} else {																							\
			sched_p.sched_active_thread = (thread_impl_t *) &sched_idle_thread;								\
		}																									\
	}																										\
																											\
	void sched_impl_yield(void) {																			\
		if ((sched_p.state & SCHED_STATUS_THREAD_COUNT_MASK) > 1) {											\
			type##_yield((sched_impl_mgr_t *) (type##_mgr_t *) &sched_p.instance);							\
			sched_p.sched_active_thread = sched_impl_active_client((type##_mgr_t *) &sched_p.instance);		\
		}																									\
	}																										\
																											\
	void sched_impl_yield_higher(void) {																	\
		if ((sched_p.state & SCHED_STATUS_THREAD_COUNT_MASK) > 1) {											\
			type##_yield_higher((sched_impl_mgr_t *) (type##_mgr_t *) &sched_p.instance);					\
			sched_p.sched_active_thread = sched_impl_active_client((type##_mgr_t *) &sched_p.instance);		\
		}																									\
	}																										\
																											\
	void sched_impl_sleep_until(unsigned int wake_time) {													\
		sleep_queue_push((sleep_queue_t *) &sched_p.sleep_mgr, 												\
						(thread_impl_t *) sched_p.sched_active_thread, wake_time);							\
		sched_impl_deregister((thread_impl_t *) sched_p.sched_active_thread);								\
	}																										\

DECLARE_SCHED_IMPL_FNS(vtrr);
