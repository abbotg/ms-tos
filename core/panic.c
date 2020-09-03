/*
 * panic.c
 *
 *  Created on: Jun 21, 2020
 *      Author: krad2
 */

#include "rtos.h"
#include "panic.h"

void __attribute__((noreturn, noinline)) panic(panic_code_t crash_code, const char *message) {
	arch_disable_interrupts();
	arch_panic(crash_code, message);
	arch_enable_interrupts();

	/* should never get here, but if it does, reboot */
	while (1) {
		arch_panic(PANIC_HARD_REBOOT, "Critical system error, rebooting");
	}
}
