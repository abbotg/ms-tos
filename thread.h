//
// Created by gta on 10/24/19.
//

#ifndef RTOS_THREAD_H
#define RTOS_THREAD_H

#include <defines.h>

/**
 * MSP430 Trapframe
 * - Generated on an interrupt
 * - Contains PC + SR
 */

typedef union __attribute__((packed)) {
	uint32_t value;
	uint16_t words[2];

	/**
	 * CPUX Trapframe (20-bit)
	 * - SP + 2: PC[15:0]
	 * - SP: PC[19:16], SR
	 *
	 * CPU Trapframe (16-bit)
	 * - SP + 2: PC[15:0]
	 * - SP: SR
	 */

	#if defined(RTOS_MSP430X_TRAPFRAME__)
		struct {
			unsigned int sr : 12;
			unsigned int pc_high : 4;
			unsigned int pc_low : 16;
		};
	#else
		struct {
			unsigned int sr : 16;
			unsigned int pc : 16;
		};
	#endif
} trapframe;

/**
 * Constructs a trapframe from a function address & a CPU status word
 */

trapframe trapframe_init(word_t pc, word_t sr);

/**
 * Thread Context
 * - Contains register states
 * - Contains trapframe for RETI from scheduler interrupt
 */

typedef struct __attribute__((packed)) {
	union {
		struct {
			word_t r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, sp;
		} registers;
		word_t regs[12];
	};

	trapframe tf;
} context;

/**
 * Thread object
 * - Contains thread context
 * - Contains stack
 */

typedef uint8_t tid_t;

typedef struct {

	// Thread context
	context ctx;

	// Process memory
	uint16_t stack[STACKSIZE];

	// Thread ID
	tid_t tid;

	struct thread *next;
	bool available;
} thread;

void thread_init(thread *this, void (*routine)(void));

word_t thread_get_sp(thread *this);
void thread_set_sp(thread *this, word_t new);

#endif //RTOS_THREAD_H
