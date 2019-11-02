/*
 * thread.c
 *
 *  Created on: Oct 31, 2019
 *      Author: krad2
 */

#include <thread.h>

trapframe trapframe_init(word_t pc, word_t sr) {
	trapframe tf;
	#if defined(RTOS_MSP430X_TRAPFRAME__)
		unsigned int pc_high_ = (pc & 0xF0000) >> 16;	// PC[19:16]
		unsigned int pc_low_ = pc & 0xFFFF;				// PC[15:0]
		unsigned int sr_ = sr & 0xFFF;					// SR

		tf.pc_high = pc_high_;
		tf.pc_low = pc_low_;
		tf.sr = sr_;
	#else
		tf.pc = pc & 0xFFFF;
		tf.sr = sr & 0xFF;
	#endif

	return tf;
}

// Returns the 'bottom' of the stack of a process
static uint16_t *stack_base(thread *this) {
	return this->stack + STACKSIZE;
}

// Initializes a process with a runnable
void thread_init(thread *this, void (*routine)(void)) {

	// Clears out all memory in the process
	memset(this, 0, sizeof(thread));

	// Trapframe + variable structure used in every scheduler invocation past the 1st
	// Need location to pop these values off and enter process with clean stack
	word_t starting_sp = (word_t) stack_base(this) - (sizeof(trapframe) + sizeof(word_t));

	// Location of trapframe in the 'dummy' frame loaded on
	word_t tf_sp = (word_t) stack_base(this) - sizeof(trapframe);

	// Initialize the trapframe
	this->ctx.tf = trapframe_init((word_t) routine, GIE);

	// Initialize the process with the stack preloaded with a trapframe
	this->ctx.registers.sp = starting_sp;

	// Copy the trapframe over to the stack (will be popped off on boot / every invocation)
	memcpy((void *) tf_sp, &this->ctx.tf, sizeof(trapframe));
}
