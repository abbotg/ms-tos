/*
 * thread_impl.c
 *
 *  Created on: Jun 26, 2020
 *      Author: krad2
 */

#include "thread_impl.h"
#include "hal.h"

void thread_impl_init(thread_impl_t *me, void *sp, thread_fn_t runnable, void *args) {
	me->sp = (void *) arch_init_stack(sp, runnable, args);
}
