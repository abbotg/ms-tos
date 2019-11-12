/*
 * os.h
 *
 *  Created on: Oct 18, 2019
 *      Author: gta
 */

#ifndef OS_H_
#define OS_H_

#include <msp430.h>

#include "defines.h"
#include "scheduler.h"
#include "config.h"

typedef int8_t thread_t; // index in tcbs array

uint16_t num_ctx_switches;

thread threads[NUMTHREADS];
unsigned run_ct;

inline void preempt_trigger(void);
inline void preempt_init(void);
void preempt_reset(void);
extern void preempt_firstrun(void);

void panic(int) __attribute__ ((noreturn));

void os_init(void);
void os_run(void) __attribute__ ((noreturn));
thread_t os_thread_create(int (*routine)(void *));
void os_yield(void);

// test func
void os_thread_set(int (*routine1)(void *), int (*routine2)(void *));


#endif /* OS_H_ */
