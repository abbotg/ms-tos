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
	return this->stack + sizeof(this->stack) / sizeof(uint16_t);
}
extern void thread_exit(int res);

// Initializes a process with a runnable
void thread_init(thread *this, int (*routine)(void *), void *arg) {

	// Clears out all memory in the process
	memset(this, 0, sizeof(thread));

	// Trapframe + variable structure used in every scheduler invocation past the 1st
	// Need location to pop these values off and enter process with clean stack

	const word_t stack_base_ = (word_t) stack_base(this);

	const word_t ret_offset = sizeof(word_t);
	const word_t tf_offset = ret_offset + sizeof(trapframe);
	const word_t tf_top_offset = tf_offset + sizeof(word_t);

	const word_t starting_sp = stack_base_ - tf_top_offset;

	// Location of trapframe in the 'dummy' frame loaded on
	const word_t tf_loc = stack_base_ - tf_offset;

	// Location of return address in the stack
	const word_t ret_loc = stack_base_ - ret_offset;

	// Initialize the trapframe
	this->ctx.tf = trapframe_init((word_t) routine, GIE);

	// Copy the trapframe over to the stack (will be popped off on boot / every invocation)
	memcpy((void *) tf_loc, &this->ctx.tf, sizeof(trapframe));

	// Copy the thread exit routine to the stack
	*((word_t *) ret_loc) = (word_t) thread_exit;

	// Initialize the process with the stack preloaded with a trapframe & R15
	this->ctx.registers.sp = starting_sp;

	// Set up argument pointer
	this->ctx.registers.r12 = (word_t) arg;
}
extern void panic(int res);
void thread_exit(int res) {
	panic(res);
}
