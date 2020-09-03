/*
 * hal.c
 *
 *  Created on: Jun 2, 2020
 *      Author: krad2
 */

#include "hal.h"

/*-----------------------------------------------------------*/

/**
 * @name Interrupt control wrappers
 * @{
 */

#define ARCH_FLAG_INTERRUPTS_ENABLED ((arch_flags_t) GIE)

inline void arch_disable_interrupts(void) {
	__disable_interrupt();	/* defined in msp430.h */
}

inline void arch_enable_interrupts(void) {
	__enable_interrupt();	/* defined in msp430.h */
}

inline void arch_set_interrupt_state(arch_flags_t mask) {
	/* erase the interrupt enable bit, then set it if appropriate */
	__bic_SR_register(GIE);
	__bis_SR_register(mask & GIE);
}

inline arch_flags_t arch_get_interrupt_state(void) {
	/* nothing but the interrupt enable bit GIE matters */
	return __get_SR_register() & GIE;
}

inline bool arch_interrupts_enabled(void) {
	return (arch_get_interrupt_state() == ARCH_FLAG_INTERRUPTS_ENABLED);
}

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name MSP430 ISA-specific stack initialization.
 * @{
 */

/**
 * @brief Arch-specific task exit handler. Captures important state parameters, then transfers control to the scheduler.
 */
static void __attribute__((naked, noreturn)) arch_task_exit(int exit_code) {
	arch_disable_interrupts();
////	sched_task_exit();
//	arch_restore_context();

	while (1) {
		panic(PANIC_UNDEFINED, "arch_task_exit() failed");
	}
}

/**
 * @brief Creates a stackframe for fast returns into tasks, emulating an interrupt.
 */
static void arch_create_irq_frame(arch_iframe_t *frame,
		thread_fn_t xcode, arch_flags_t cpu_flags) {

	/* 20-bit MSP430s concatenate the SR with the 20-bit PC in a 32-bit trapframe */
	#ifdef __MSP430X_LARGE__

		/* Split 20-bit function address into 16-bit and 4-bit segments for appropriate placement */
		const arch_reg_t addr_runnable = (arch_reg_t) xcode;
		const uint16_t addr_runnable_l = addr_runnable & 0x0000FFFF;
		const uint16_t addr_runnable_h = (addr_runnable & 0x000F0000) >> 16;

		/**
		 * The highest word address contains the lower 16-bits of the PC in LE format.
		 * The lowest word address contains the SR at the lowest byte.
		 * The other byte has the PC top bits at the high nibble, with other nibble being reserved.
		 */

		/* Highest word address */
		frame->words[1] = addr_runnable_l;

		/* Lowest word & byte address */
		frame->bytes[0] = (uint8_t) cpu_flags;

		/* The remaining byte has the PC at the high nibble */
		frame->bytes[1] = (uint8_t) (addr_runnable_h << 4);
	#else
		/* 16-bit MSP430s store the SR at the next word below the function address in the same size */
		frame->words[1] = (uint16_t) xcode;
		frame->words[0] = (uint8_t) cpu_flags;
	#endif
}

/**
 * @brief Sets up a task stack for thread execution.
 */
volatile arch_reg_t *arch_init_stack(volatile arch_reg_t *stack_top,
		volatile thread_fn_t xcode, volatile void *fn_args) {

	/* Precondition: stack_top points to the bottom of the task stack. */

	/**
	 * Preallocate a constant number of bytes for the interrupt frame,
	 * then create the frame centered on that memory space at stack_top.
	 */
	const uint8_t *stack_base = (uint8_t *) stack_top;

	/* allocate a word for the exit handler */
	stack_top--;
	*stack_top = (arch_reg_t) arch_task_exit;

	/* allocate an interrupt frame for context switching */
	stack_top = (arch_reg_t *) (stack_base - sizeof(arch_iframe_t));
	arch_create_irq_frame((arch_iframe_t *) stack_top, xcode, GIE);

	/**
	 * Next, preallocate 1 register at a time and preload it with a value.
	 * R12 is an exception because the ABI specifies it as the 1st argument register.
	 */

	stack_top--;							/* Preallocate R15 */
	*stack_top = (arch_reg_t) 0xeeeee; 		/* Preload R15 to 0xEEEEE */

	stack_top--;
	*stack_top = (arch_reg_t) 0xddddd;

	stack_top--;
	*stack_top = (arch_reg_t) 0xccccc;

	stack_top--;							/* Preallocate R12 */
	*stack_top = (arch_reg_t) fn_args;		/* Preload R12 to fn_args */

	stack_top--;
	*stack_top = (arch_reg_t) 0xbbbbb;

	stack_top--;
	*stack_top = (arch_reg_t) 0xaaaaa;

	stack_top--;
	*stack_top = (arch_reg_t) 0x99999;

	stack_top--;
	*stack_top = (arch_reg_t) 0x88888;

	stack_top--;
	*stack_top = (arch_reg_t) 0x77777;

	stack_top--;
	*stack_top = (arch_reg_t) 0x66666;

	stack_top--;
	*stack_top = (arch_reg_t) 0x55555;

	/* Last register is R4 at the top of the stack. */
	stack_top--;
	*stack_top = (arch_reg_t) 0x44444;

	return stack_top;
}

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name MSP430 hardware-specific OS feature configuration.
 * @{
 */

/* MSP430 hardware timer-specific interrupt vectors */
#define ARCH_TICK_VECTOR 								TIMER0_A0_VECTOR
#define ARCH_TIMEKEEPING_VECTOR							TIMER0_A1_VECTOR

/* Attributes of our timer setup */
#define ARCH_TICK_CLK_FREQ								4096
#define ARCH_TIME_MAX_SECONDS							(unsigned int) (65536.0 / ARCH_TICK_CLK_FREQ)

/* Equivalent period for the configured tick rate */
#define CONFIG_TICK_RATE_MS								(unsigned int) (1000.0 / CONFIG_TICK_RATE_HZ)

/* Calculate hardware parameters to achieve the constants above, with or w / o approximation */
#if (CONFIG_USE_FAST_MATH == 1)
	#define ARCH_1MS									((ARCH_TICK_CLK_FREQ) >> 10)
	#define ARCH_MS_TO_CYCLES(ms)						((((uint32_t) ARCH_TICK_CLK_FREQ) * ((uint32_t) ms)) >> 10)
#else
	#define ROUND(x) 									((x) >= 0 ? (long) ((x) + 0.5) : (long) ((x) - 0.5))
	#define ARCH_1MS									(double) (ARCH_TICK_CLK_FREQ / 1000.0)
	#define ARCH_MS_TO_CYCLES(ms)						ROUND(((double) ms) * ARCH_1MS)
#endif

/**
 * @brief Sets up OS time slicing interrupt.
 * @param[in] ms period of timer interrupt, measured in cycles.
 */
static void arch_setup_timer_interrupt(unsigned int period) {
	TA0CTL = MC_0 | TACLR;
	TA0CCR0 = period;
	TA0CCTL0 = CCIE;
	TA0CTL = TASSEL_1 | ID_3 | MC_2;
}

static void arch_disable_timer_interrupt(void) {
	TA0CTL = MC_0 | TACLR;
}

/**
 * @brief Acknowledges a wakeup interrupt, but doesn't stop the system timer.
 */
static void arch_acknowledge_wakeup_interrupt() {
	TA0CCR1 &= ~CCIFG;
}

/**
 * @brief Acknowledges a tick interrupt, but doesn't stop the tick timer.
 */
static void arch_acknowledge_tick_interrupt() {
	TA0CCR0 &= ~CCIFG;
}

/**
 * @brief Sets up an interrupt at the specified time.
 * @param[in] next_wake_time When to expect a thread awakening, measured in cycles.
 */
static void arch_schedule_next_wakeup(unsigned int next_wake_time) {
	TA0CCR1 = next_wake_time;
	TA0CCTL1 |= CCIE;
}

/**
 * @brief Sets up an interrupt so the next context switch will occur.
 * @param[in] next_wake_time When to expect a thread awakening, measured in cycles.
 */
static void arch_schedule_next_tick(unsigned int next_tick_time) {
	TA0CCR0 = TA0R + next_tick_time;
	TA0CCTL0 |= CCIE;
}

/**
 * @brief Masks tick interrupts, but doesn't stop the tick timer.
 */
static void arch_suppress_wakeup_interrupt() {
	TA0CCR1 &= ~CCIE;
}

/**
 * @brief Masks wakeup interrupt, but doesn't stop the time keeper.
 */
static void arch_suppress_tick_interrupt() {
	TA0CCR0 &= ~CCIE;
}

#if (CONFIG_WATCHDOG_MONITOR == 1)

/**
 * @brief Sets up the watchdog for the watchdog monitor.
 */
static void arch_setup_watchdog_interrupt(void) {
	WDTCTL = WDT_ARST_1000;
	SFRIE1 |= WDTIE;
}

#endif

unsigned int arch_time_now(void) {
	return TA0R;
}

void arch_idle(void) {
	_low_power_mode_3();
}

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name MSP430 ISA-specific scheduler initialization (and preparations for arch_sched_end()).
 * @{
 */

/**
 * @brief Boots up the scheduler and saves pre-boot state for quitting.
 */
void __attribute__((noinline)) arch_sched_start(void) {
	arch_disable_interrupts();

	/* Back up the context before booting up the kernel so that we can return later. */
	arch_save_regs();
	#ifdef __MSP430X_LARGE__
		__asm__ __volatile__("mov.a sp, %0" : "=r"(sched_p.boot_context));
	#else
		__asm__ __volatile__("mov.w sp, %0" : "=r"(sched_p.boot_context));
	#endif

	/* Set up the time slicing and safety hardware. */
	arch_setup_timer_interrupt(ARCH_MS_TO_CYCLES(CONFIG_TICK_RATE_MS));
	#if (CONFIG_WATCHDOG_MONITOR == 1)
		arch_setup_watchdog_monitor();
	#endif

	/* Find and return into the first task. */
	sched_impl_run();
	arch_restore_context();
}

/**
 * @brief Exits all threads to the pre-booting location.
 */
void __attribute__((naked)) arch_sched_end(void) {

	/* Disable all preemption. */
	arch_disable_interrupts();
	arch_disable_timer_interrupt();

	/* Reload the pre-boot state so we can return back to the OS spawn point. */
	#ifdef __MSP430X_LARGE__
		__asm__ __volatile__("mov.a %0, sp" : : "m"(sched_p.boot_context));
	#else
		__asm__ __volatile__("mov.w %0, sp" : : "m"(sched_p.boot_context));
	#endif
	arch_restore_regs();

	/**
	 * After restoring registers, the noinline'd arch_sched_start() should have definitely
	 * placed a return address on the stack that we can directly return into.
	 */
	#ifdef __MSP430X_LARGE__
		__asm__ __volatile__("reta");
	#else
		__asm__ __volatile__("ret");
	#endif
}

/** @} */

/*-----------------------------------------------------------*/

/**
 * @brief Kernel panic function.
 */
static char panic_log[CONFIG_PANIC_DUMP_SIZE];
void __attribute__((noinline)) arch_panic(panic_code_t crash_code, const char *message) {

	// double pop address to get the source of the panic

	/* Handle the crash reasons differently. */
	switch (crash_code) {
		case PANIC_GENERAL_ERROR: break;
		case PANIC_SOFT_REBOOT: break;
		case PANIC_HARD_REBOOT: break;
		case PANIC_ASSERT_FAIL: break;
		case PANIC_EXPECT_FAIL: break;
		case PANIC_SSP: break;
		case PANIC_UNDEFINED: break;
	}

	/* Depending on configuration, either halt or reset. */
	#if (CONFIG_DEBUG_MODE == 1)
		while (1) __bis_SR_register(LPM4_bits & ~GIE);
	#else
		arch_system_reset();
	#endif
}

/*-----------------------------------------------------------*/

/**
 * @name MSP430 task switching intrinsics.
 * @{
 */

/**
 * Yield needs to emulate the stacking behavior of a preemptive tick.
 * Since a tick pushes the PC + SR on the stack, we also need to do that.
 * As this it is a function call, we have to manually push the SR.
 * The rest reads like a standard context switch.
 * Therefore, the following helper functions are defined to take care of this.
 */
static inline void __attribute__((always_inline)) arch_context_switch_prologue(void) {

	#ifdef __MSP430X_LARGE__
		/* Save the SR */
		__asm__ __volatile__("mov.b sr, 3(sp)");

		/* PC + SR is stacked so interrupts are disabled */
		__disable_interrupt();
	#else
		__asm__ __volatile__("push sr");
		__disable_interrupt();
	#endif
}

/**
 * @brief Performs a manual context switch emulating a hardware context switch.
 */
static inline void __attribute__((always_inline)) arch_build_trapframe(void) {

	/**
	 * The 20-bit trapframe is bizarre as previously outlined,
	 * so additional work needs to be done to format a normal function address into
	 * something RETI can work with.
	 *
	 * The 16-bit trapframe doesn't need any work at all.
	 */
	#ifdef __MSP430X_LARGE__

		/* R12 is used as a temp variable so it needs to be backed up. */
		__asm__ __volatile__("pushx.a r12");

		/* Shifts upper 4 bits of PC to higher nibble at SP + 2. */
		__asm__ __volatile__("mov.b 6(sp), r12");
		__asm__ __volatile__("rpt #4");
		__asm__ __volatile__("rlax.b r12");
		__asm__ __volatile__("mov.b r12, 6(sp)");

		/* Swap PC and SR bytes so that the SR moves to a lower address */
		__asm__ __volatile__("swpb 6(sp)");

		/**
		 * Swap adjacent words so that (PC + SR) moves to the lowest address
		 * and the low bits of the PC are at higher addresses.
		 */
		__asm__ __volatile__("mov.w 6(sp), r12");
		__asm__ __volatile__("mov.w 4(sp), 6(sp)");
		__asm__ __volatile__("mov.w r12, 4(sp)");

		/* Restore R12, we don't need it anymore. */
		__asm__ __volatile__("popx.a r12");

	#endif
}

/**
 * @brief Changes to any other runnable thread.
 */
void __attribute__((noinline, naked)) arch_yield(void) {

	/* Manual context stacking. */
	arch_context_switch_prologue();
	arch_build_trapframe();

	/* Standard context switching logic. */
	arch_save_context();

	/* Check for stack overflow. */
	#if (CONFIG_CHECK_FOR_STACK_OVERFLOW == 1)

	#endif

	/* Find a new thread of any priority. */
	sched_impl_yield();

	/* Check if the newly scheduled thread holds a critical section, and restore the lock if true. */
	if (sched_p.state & SCHED_STATUS_IRQ_LOCKED) {

		/* We need to modify the stack, so we need to get the location of the interrupt status register. */
		const register volatile uint8_t *stack_top = (uint8_t *) sched_p.sched_active_thread->sp;
		const register unsigned int iframe_offs = offsetof(arch_context_t, task_addr);
		register volatile uint8_t *arch_iframe_pos = (uint8_t *) (stack_top + iframe_offs);

		/**
		 * Disabling interrupts will allow the new thread to proceed uninterrupted
		 * since it must have yielded in a critical section.
		 */
		*arch_iframe_pos &= ~ARCH_FLAG_INTERRUPTS_ENABLED;
	}

	/* Return into the thread. */
	arch_restore_context();
}

/**
 * @brief Changes to a higher priority thread.
 */
void __attribute__((noinline, naked)) arch_yield_higher(void) {

	/* Manual context stacking. */
	arch_context_switch_prologue();
	arch_build_trapframe();

	/* Standard context switching logic. */
	arch_save_context();

	/* Check for stack overflow. */
	#if (CONFIG_CHECK_FOR_STACK_OVERFLOW == 1)

	#endif

	/* Find a higher priority thread and return into it if possible. */
	sched_impl_yield_higher();

	/* Check if the newly scheduled thread holds a critical section, and restore the lock if true. */
	if (sched_p.state & SCHED_STATUS_IRQ_LOCKED) {

		/* We need to modify the stack, so we need to get the location of the interrupt status register. */
		const register volatile uint8_t *stack_top = (uint8_t *) sched_p.sched_active_thread->sp;
		const register unsigned int iframe_offs = offsetof(arch_context_t, task_addr);
		register volatile uint8_t *arch_iframe_pos = (uint8_t *) (stack_top + iframe_offs);

		/**
		 * Disabling interrupts will allow the new thread to proceed uninterrupted
		 * since it must have yielded in a critical section.
		 */
		*arch_iframe_pos &= ~ARCH_FLAG_INTERRUPTS_ENABLED;
	}

	/* Return into the thread. */
	arch_restore_context();
}

/**
 * @brief Puts the current thread to sleep by scheduling a wakeup at wake_time.
 * @param[in] wake_time the time, in cycles, that the thread will be put back on the run queue.
 */
static void arch_sleep_until(unsigned int wake_time) {
	thread_impl_t *next_waker = sleep_queue_peek(&sched_p.sleep_mgr);
	arch_schedule_next_wakeup(next_waker->sq_entry.wake_time);

	arch_yield();
}

/**
 * @brief Puts the thread to sleep for the given period of time.
 * @param[in] ms How long to sleep for, in milliseconds.
 */
void arch_sleep_for(unsigned int ms) {
	unsigned int now = arch_time_now();
	unsigned int wake_time = now + ARCH_MS_TO_CYCLES(ms);

	sched_impl_sleep_until(wake_time);
	arch_sleep_until(wake_time);
}


/**
 * @brief Scheduler preemption tick. Invokes sched_run() to distribute time slices.
 */
__attribute__((naked, interrupt(ARCH_TICK_VECTOR))) void arch_tick_irq(void) {

	/* Standard context switching logic. */
	arch_save_context();

	arch_acknowledge_tick_interrupt();

	profile_start();

	/* Check for stack overflow. */
	#if (CONFIG_CHECK_FOR_STACK_OVERFLOW == 1)

	#endif

	/* Find the next logical thread in the sequence. */
	sched_impl_run();

	/* Figure out when to service the next interrupt(s). */
	#if (CONFIG_USE_TICKLESS_IDLE == 1)

		/* If there are sleeping threads that need to be awoken, schedule the next service time. */
//		thread_impl_t *next_waker = sleep_queue_peek(&sched_p.sleep_mgr);
//		if (next_waker != NULL) arch_schedule_next_wakeup(next_waker->sq_entry.wake_time);
//		else arch_suppress_wakeup_interrupt();

		/* If there is more than 1 thread, attention needs to be divided, so keep timeslicing. */
		if ((sched_p.state & SCHED_STATUS_THREAD_COUNT_MASK) > 1) arch_schedule_next_tick(ARCH_MS_TO_CYCLES(CONFIG_TICK_RATE_MS));
		else arch_suppress_tick_interrupt();
	#else

		/* Service sleeping threads in software. */
		// check TA0R against the min every tick and perform wakeup() manually if true
		arch_schedule_next_tick(CONFIG_TICK_RATE_MS);
	#endif

	profile_end();
	arch_restore_context();
}

__attribute__((naked, interrupt(ARCH_TIMEKEEPING_VECTOR))) void arch_time_irq(void) {
	arch_enter_isr();

	switch (__even_in_range(TA0IV, TA0IV_TAIFG)) {

		/* We shouldn't be able to trap into an interrupt without the hardware knowing. */
		case TA0IV_NONE:
			panic(PANIC_EXPECT_FAIL, "Unexpected trap into ARCH_TIMEKEEPING_VECTOR");
			break;

		/* Whenever a wakeup happens. */
		case TA0IV_TACCR1:

			/* Necessary so we don't get in a loop here. */
			arch_acknowledge_wakeup_interrupt();

			/* First get the thread that needs to be awoken right now and add it back to the run queue. */
			volatile thread_impl_t *waker = sleep_queue_peek(&sched_p.sleep_mgr);
			sleep_queue_pop(&sched_p.sleep_mgr);
			sched_impl_register(waker);

			/**
			 * Next find out who's next on the list, and if there is no one on the list, turn off the
			 * wakeup interrupt. Otherwise, configure the wakeup interrupt for the entry's stated wakeup time.
			 */
			volatile thread_impl_t *next_waker = sleep_queue_peek(&sched_p.sleep_mgr);
			if (next_waker == 0) arch_suppress_wakeup_interrupt();
			else arch_schedule_next_wakeup(next_waker->sq_entry.wake_time);
			break;

		/* Unused for now, so these are all unexpected traps. */
		case TA0IV_TACCR2:
			panic(PANIC_EXPECT_FAIL, "Unexpected trap into ARCH_TIMEKEEPING_VECTOR");
			break;

		case TA0IV_TACCR3:
			panic(PANIC_EXPECT_FAIL, "Unexpected trap into ARCH_TIMEKEEPING_VECTOR");
			break;

		case TA0IV_TACCR4:
			panic(PANIC_EXPECT_FAIL, "Unexpected trap into ARCH_TIMEKEEPING_VECTOR");
			break;

		case TA0IV_5:
			panic(PANIC_EXPECT_FAIL, "Unexpected trap into ARCH_TIMEKEEPING_VECTOR");
			break;

		case TA0IV_6:
			panic(PANIC_EXPECT_FAIL, "Unexpected trap into ARCH_TIMEKEEPING_VECTOR");
			break;

		case TA0IV_TAIFG:
			panic(PANIC_EXPECT_FAIL, "Unexpected trap into ARCH_TIMEKEEPING_VECTOR");
			break;

		default:
			break;
	}

	arch_exit_isr();
}

/** @} */
