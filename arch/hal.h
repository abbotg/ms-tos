/*
* hal.h
*
*  Created on: Jun 2, 2020
*      Author: krad2
*/

#ifndef ARCH_HAL_H_
#define ARCH_HAL_H_

#include <msp430.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "rtos.h"
#include "internals.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------*/

/**
 * @name Hardware-specific register definitions
 * @{
 */

/**
 * @brief The abstract arch register's underlying data type changes based on the code / data model.
 */

typedef uintptr_t arch_reg_t;

/**
 * @brief The abstract arch status register's underlying data type is 8 bits for MSP430.
 */
typedef uint8_t arch_flags_t;

/**
 * @brief The abstract interrupt hardware stack frame is 4 bytes but the contents depend on code / data model.
 */
typedef union arch_iframe {
	uint8_t bytes[4];
	uint16_t words[2];
} arch_iframe_t;

/**
 * @brief The layout of thread context as seen on the stack.
 */

typedef struct arch_task_context {
	arch_reg_t r4;				/* top of stack */
	arch_reg_t r5;
	arch_reg_t r6;
	arch_reg_t r7;
	arch_reg_t r8;
	arch_reg_t r9;
	arch_reg_t r10;
	arch_reg_t r11;
	arch_reg_t r12;
	arch_reg_t r13;
	arch_reg_t r14;
	arch_reg_t r15;
	arch_iframe_t task_addr;	/* return address into the task */
	arch_reg_t task_exit;		/* return address for task deletion */
} arch_context_t;

/**
 * @brief Underlying data type used for timekeeping.
 */
#if (CONFIG_USE_16_BIT_TICKS == 1)
	typedef uint16_t arch_tick_t;
#else
	typedef uint32_t arch_tick_t;
#endif

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name OS-aware ISR helpers
 * @{
 */

/**
 * @def ISR
 * @brief Defines an OS-aware ISR.
 * @details Must be wrapped by calls to arch_enter_isr() and arch_exit_isr().
 * @param[in] vector	ISR vector number for the ISR table.
 * @param[in] fn		The name of the function being connected to the ISR.
 */
#define ISR(vector, fn)       __attribute__((naked, interrupt(vector))) void fn(void)

/**
 * @brief ISR entry hook. Switches to a kernel interrupt stack if appropriate, then sets IRQ_IN.
 */
static inline void __attribute__((always_inline)) arch_enter_isr(void) {

	/* back up all context on the interrupted task stack */
	arch_save_context();

	/* changing to a separate kernel interrupt stack reduces stack overflow potential */
	#if (CONFIG_USE_KERNEL_STACK == 1)
		#ifdef __MSP430X_LARGE__
				__asm__ __volatile__("mov.a %0, sp" : : "i"(sched_g.sched_isr_stack + CONFIG_ISR_STACK_SIZE));
		#else
				__asm__ __volatile__("mov.w %0, sp" : : "i"(sched_g.sched_isr_stack + CONFIG_ISR_STACK_SIZE));
		#endif
	#endif

	/* notify that we're in an IRQ */
	sched_set_status(STATUS_IN_IRQ);
}

/**
 * @brief ISR exit hook. Clears IRQ_IN, and yields to a higher priority thread if appropriate.
 */
static inline void __attribute__((always_inline)) arch_exit_isr(void) {

	/* notify that the IRQ is done */
	sched_clear_status(STATUS_IN_IRQ);

	/* if the interrupt awakened a high priority thread, select that for context switch */

	if (sched_get_status() & STATUS_CONTEXT_SWITCH_REQUEST) {
		sched_run();
	}

    /**
     * if a new task was chosen, switch into that one.
     * else switch back into the interrupted stack
     * because sched_active_thread was already updated.
     */
    arch_restore_context();
}

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name Interrupt control helpers
 * @{
 */

/**
 * @brief Sets the IRQ disable bit in the status register.
 */
void arch_disable_interrupts(void);

/**
 * @brief Clears the IRQ disable bit in the status register.
 */
void arch_enable_interrupts(void);

/**
 * @brief Sets the interrupt control flag to the specified value.
 * @param[in] mask	IRQ state to restore to.
 */
void arch_set_interrupt_state(arch_flags_t mask);

/**
 * @brief Retrieves the value of the IRQ status bit from the status register.
 * @return Value of the status register. Should not be interpreted as a boolean and instead as the raw data.
 * @see arch_interrupts_enabled
 */
arch_flags_t arch_get_interrupt_state(void);

/**
 * @brief Checks if interrupts are currently enabled or disabled. Invokes arch_get_interrupt_state().
 * @return True if interrupts are active, false if not.
 */
bool arch_interrupts_enabled(void);

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name Interrupt control helpers
 * @{
 */

void arch_panic(panic_code_t crash_code, const char *message);

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name Task and scheduler contract functions
 * @{
 */

/**
 * @brief Configures the stack of a task to look exactly as if a call to arch_save_context() was made.
 * @param[in] ptr_stack_top		Pointer to the current top of the task stack.
 * @param[in] ptr_xcode			Pointer to the thread runnable.
 * @param[in] ptr_fn_args		Pointer to the runnable function arguments.
 */
arch_reg_t *arch_init_stack(arch_reg_t *stack_top, thread_fn_t xcode, void *fn_args);

/**
 * @brief Sets up the hardware time slicer.
 * @details Not implemented. Must be provided by user.
 */
void arch_setup_timer_interrupt(void);

/**
 * @brief Stops the hardware time slicer.
 * @details Not implemented. Must be provided by user.
 */
void arch_disable_timer_interrupt(void);

/**
 * @brief Starts the OS scheduler. Invokes arch_setup_timer_interrupt().
 */
void __attribute__((noinline)) arch_sched_start(void);

/**
 * @brief Exits the currently running thread and disables the scheduler. Invokes arch_disable_timer_interrupt().
 */
void __attribute__((naked)) arch_sched_end(void);

/**
 * @brief Manual context switch. Surrenders time slice to the next thread in the run queue.
 */
void __attribute__((noinline, naked)) arch_yield(void);

/**
 * @brief Manual context switch. Surrenders time slice to the highest priority thread in the run queue.
 */
void __attribute__((noinline, naked)) arch_yield_higher(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ARCH_HAL_H_ */
