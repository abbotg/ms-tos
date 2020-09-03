/*
 * port_config.h
 *
 *  Created on: May 23, 2020
 *      Author: krad2
 */

#ifndef PORT_CONFIG_H_
#define PORT_CONFIG_H_

#define CONFIG_NUM_COOP_PRIORITIES
#define CONFIG_NUM_PREEMPT_PRIORITIES
#define CONFIG_PREEMPT_THRESHOLD

#define CONFIG_USE_16_BIT_TICKS 									1
#define CONFIG_USE_TICKLESS_IDLE									1
#define CONFIG_TICK_RATE_HZ											100

#define CONFIG_USE_KERNEL_STACK										1
#define CONFIG_ISR_STACK_SIZE 										256
#define CONFIG_IDLE_STACK_SIZE										128

// static allocation requires a preallocated heap buffer with a certain size
#define CONFIG_STATIC_ALLOCATION
#define CONFIG_STATIC_HEAP_SIZE

#define CONFIG_DYNAMIC_ALLOCATION

#define CONFIG_SCHED_RR												0
#define CONFIG_SCHED_VTRR 											1
#define CONFIG_SCHED_LOTTERY										0
#define CONFIG_SCHED_MULTIQ											0

#define CONFIG_USE_IDLE_HOOK                     					0
#define CONFIG_USE_TICK_HOOK                     					0
#define CONFIG_CHECK_FOR_STACK_OVERFLOW          					0

#define CONFIG_DEBUG_MODE											1
#define CONFIG_BENCHMARK_MODE										1
#define CONFIG_USE_FAST_MATH										1

#define CONFIG_WATCHDOG_MONITOR										0

#define CONFIG_PANIC_DUMP_SIZE										128

// priority inheritance should force a non-dumb wait queue implementation

#endif /* PORT_CONFIG_H_ */
