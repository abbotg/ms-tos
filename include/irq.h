/*
 * irq.h
 *
 *  Created on: Jun 21, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_IRQ_H_
#define INCLUDE_IRQ_H_

#include <stdbool.h>
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int irq_lock_t;

void irq_disable(void);
void irq_enable(void);

void irq_lock(void);
void irq_unlock(void);

inline bool irq_is_in(void);
inline bool irq_locked(void);

/**
 * @def ISR
 * @brief Defines an OS-aware ISR.
 * @details Must be wrapped by calls to arch_enter_isr() and arch_exit_isr().
 * @param[in] vector	ISR vector number for the ISR table.
 * @param[in] fn		The name of the function being connected to the ISR.
 */
#define ISR(vector, fn)       __attribute__((naked, interrupt(vector))) void fn(void)

static inline __attribute__((always_inline)) enter_isr(void) {
	arch_enter_isr();
}

static inline __attribute__((always_inline)) exit_isr(void) {

}

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_IRQ_H_ */
