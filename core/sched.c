/*
 * sched.c
 *
 *  Created on: Jun 4, 2020
 *      Author: krad2
 */

#include "rtos.h"
#include "sched_impl.h"
#include "sched.h"

void sched_init(void) {
	irq_disable();
	sched_impl_init();
	irq_enable();
}

void sched_add(volatile thread_t *new, volatile unsigned int priority) {
	irq_lock();
	sched_impl_add((thread_impl_t *) &new->base, priority);
	irq_unlock();
}

void sched_register(volatile thread_t *new) {
	irq_lock();
	sched_impl_register((thread_impl_t *) &new->base);
	irq_unlock();
}

void sched_deregister(volatile thread_t *new) {
	irq_lock();
	sched_impl_deregister((thread_impl_t *) &new->base);
	irq_unlock();
}

void sched_reregister(volatile thread_t *new, volatile unsigned int priority) {
	irq_lock();
	sched_impl_reregister((thread_impl_t *) &new->base, priority);
	irq_unlock();
}

void sched_start(void) {
	irq_disable();
	sched_impl_start();
	arch_sched_start();
}

void sched_end(void) {
	irq_disable();
	sched_impl_end();
	arch_sched_end();
}

void sched_yield(void) {
	irq_lock();
	arch_yield();
	irq_unlock();
}

void sched_yield_higher(void) {
	irq_lock();
	arch_yield_higher();
	irq_unlock();
}

void sched_sleep(unsigned int ms) {
	irq_lock();
	arch_sleep_for(ms);
	irq_unlock();
}

sched_status_t sched_get_status(void) {
//	arch_disable_interrupts();
//	sched_status_t state = sched_g.state;
//	bool in_irq = (state & SCHED_STATUS_IN_IRQ) != 0;
//	bool locked = (state & SCHED_STATUS_IRQ_LOCKED) != 0;
//	if (!locked && !in_irq) arch_enable_interrupts();
	return true;;
}

void sched_set_status(sched_status_t status) {
//	sched_g.state |= status;
}

void sched_clear_status(sched_status_t status) {
//	sched_g.state &= ~status;
}

unsigned int sched_thread_count(void) {
//	irq_lock();
//	unsigned int count = (sched_g.state & SCHED_STATUS_THREAD_COUNT_MASK) >> SCHED_STATUS_THREAD_COUNT_POS;
//	irq_unlock();

	return 1;
}

thread_t *sched_current_thread() {
//	irq_lock();
//	thread_t *curr = sched_g.sched_active_thread;
//	irq_unlock();

//	return curr;
}
