/*
 * irq.c
 *
 *  Created on: Jun 22, 2020
 *      Author: krad2
 */

#include "rtos.h"
#include "sched_impl.h"
#include "irq.h"

inline void irq_disable(void) {
	arch_disable_interrupts();
}

inline void irq_enable(void) {
	arch_enable_interrupts();
}

void irq_lock(void) {
	if (arch_interrupts_enabled()) {
		arch_disable_interrupts();
		container_of(sched_p.sched_active_thread, thread_t, base)->cs_lock = 0;
//		sched_set_status(SCHED_STATUS_IRQ_LOCKED);
	}

	container_of(sched_p.sched_active_thread, thread_t, base)->cs_lock++;
}

void irq_unlock(void) {
	container_of(sched_p.sched_active_thread, thread_t, base)->cs_lock--;
	if (container_of(sched_p.sched_active_thread, thread_t, base)->cs_lock == 0) {
		container_of(sched_p.sched_active_thread, thread_t, base)->cs_lock = 1;
//		sched_clear_status(SCHED_STATUS_IRQ_LOCKED);
		arch_enable_interrupts();
	}
}

bool irq_locked(void) {
//	arch_disable_interrupts();
//	bool irq_lock_state = (sched_get_status() & SCHED_STATUS_IRQ_LOCKED) != 0;
//	if (!irq_lock_state) arch_enable_interrupts();
	return true;
//	return irq_lock_state;
}

bool irq_is_in(void) {
	arch_disable_interrupts();
	bool istate = (sched_get_status() & SCHED_STATUS_IN_IRQ) != 0;
	if (!istate) arch_enable_interrupts();

	return istate;
}

