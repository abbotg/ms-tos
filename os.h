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

#include "user.h"
#include "defines.h"
#include "config.h"
#include "scheduler.h"

/*
 * Types
 */

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


#define THRD_AVAIL_BIT__ 0
#define THRD_SEM_BIT__ 1

union thread_flags {
  uint8_t raw;
  struct {
    unsigned available : 1;
    unsigned finished_semaphore : 1;
  };
};

/**
 * Thread object
 * - Contains thread context
 * - Contains stack
 */
struct thread {
  context_t ctx; // Thread context
  trapframe_t tf;
  uint16_t stack[STACKSIZE]; // Process memory
  tid_t tid; // Thread ID
  struct thread *next;
  union thread_flags flags;
};



/*
 * Global variables
 */
uint16_t num_ctx_switches;
unsigned run_ct;
struct thread *run_ptr; // currently running thread

/*
 * Function defs
 */

/* scheduler impl (defined in config.h) */
extern void schedule(void); // called in the context switcher
extern struct thread *sched_new(void); // Retrieve a new struct thread
extern void sched_add(struct thread *); // Mark a thread as runnable
extern void sched_start(void); // Start preemption if not running already
extern void sched_init(void); // first call upon interaction with scheduler

/* preempt.c */
inline void preempt_trigger(void);
inline void preempt_init(void);
void preempt_reset(void);

/* os.c */
void panic(int) __attribute__ ((noreturn));
uint16_t *stack_base(struct thread *this);

/* osasm.s */
extern void context_save(void *);
extern void context_load(void *);
extern void preempt_firstrun(void);

/* threads.c */
void thread_fg_set(struct thread *thr, unsigned bit, bool val);
inline bool thread_fg_get(struct thread *thr, unsigned bit);

#endif /* OS_H_ */
