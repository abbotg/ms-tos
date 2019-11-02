/*
 * os.h
 *
 *  Created on: Oct 18, 2019
 *      Author: gta
 */

#ifndef OS_H_
#define OS_H_

/*
 * This file contains no userspace functionality
 * and is for OS purposes only.
 */

#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include "semaphore.h"
#include "defines.h"
#include "scheduler.h"
#include "config.h"
#include "threads.h"

// TODO: can be packed into an int
static enum {
  SYS_UNINITIALIZED = 0,
  SYS_RUNNING = 1,
  SYS_IDLE = 2
} status = SYS_UNINITIALIZED;

/**
 * MSP430 Trapframe
 * - Generated on an interrupt
 * - Contains PC + SR
 *
 * CPUX Trapframe (20-bit)
 * - SP + 2: PC[15:0]
 * - SP: PC[19:16], SR
 *
 * CPU Trapframe (16-bit)
 * - SP + 2: PC[15:0]
 * - SP: SR
 */
union trapframe {
  uint32_t value;
  uint16_t words[2];

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
};
typedef union trapframe trapframe_t;

/**
 * Constructs a trapframe from a function address & a CPU status word
 */
trapframe_t trapframe_init(word_t pc, word_t sr);

/**
 * Thread Context
 * - Contains register states
 * - Contains trapframe for RETI from scheduler interrupt
 */
union context {
  struct {
    word_t r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, sp;
  };
  word_t regs[12];
};
typedef union context context_t;

/**
 * Thread object
 * - Contains thread context
 * - Contains stack
 */
typedef uint8_t tid_t;

struct thread {
  context_t ctx; // Thread context
  trapframe_t tf;
  uint16_t stack[STACKSIZE]; // Process memory
  tid_t tid; // Thread ID
  struct thread *next;
  bool available;
  bsem_t finish_sem; // initially zero, waited on by thrd_join and posted by thrd_exit
                     // TODO: finish_sem needs to be automatically posted on thread exit
};


uint16_t num_ctx_switches;

struct thread threads[NUMTHREADS];
unsigned run_ct;

inline void preempt_trigger(void);
inline void preempt_init(void);
void preempt_reset(void);
extern void preempt_firstrun(void);

void panic(int) __attribute__ ((noreturn));

void os_init(void);
void os_run(void) __attribute__ ((noreturn));


void os_thread_set(void (*routine1)(void), void (*routine2)(void)); // TODO: remove

uint16_t *stack_base(struct thread *this);

// osasm.s
extern void context_save(void *);
extern void context_load(void *);

#endif /* OS_H_ */
