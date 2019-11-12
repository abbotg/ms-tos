/*
 * os.c
 *
 *  Created on: Oct 18, 2019
 *      Author: gta
 */
#include "os.h"

thread_t
os_thread_create(int (*routine)(void *))
{
  int i;
  __disable_interrupt();
  for (i = 0; i < NUMTHREADS; ++i) {
    if (threads[i].available) {
      threads[i].available = false;
      thread_init(threads + i, routine, NULL);
      (void) link();
      __enable_interrupt();
      return i;
    }
  }
  __enable_interrupt();
  return -1;
}

void
os_init(void)
{
  int i;
  for (i = 0; i < NUMTHREADS; ++i)
    threads[i].available = 0;
}

void
os_run(void)
{
  preempt_firstrun();
  panic(0);
}

void
os_yield(void)
{
  preempt_trigger();
}

volatile int q = 0;

void
os_thread_set(int (*routine1)(void *),
              int (*routine2)(void *))
{

	thread_init(threads + 0, routine1, (void *) &q);
	thread_init(threads + 1, routine2, (void *) &q);

	threads[0].next = &threads[1];
	threads[1].next = &threads[0];

	run_ptr = &threads[0];
}


void
panic(c)
{
	__disable_interrupt();
  volatile int code = c; // for debug
  volatile thread *run_pt = run_ptr;
  for (;;)
    ;
}
