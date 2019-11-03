/*
 * os.c
 *
 *  Created on: Oct 18, 2019
 *      Author: gta
 */
#include "os.h"

trapframe_t
trapframe_init(word_t pc, word_t sr)
{
  trapframe_t tf;
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
uint16_t *
stack_base(struct thread *this)
{
  return this->stack + STACKSIZE;
}


void
os_init(void)
{
  int i;
  __disable_interrupt(); // disable interrupts until os_run
  preempt_reset();
  for (i = 0; i < NUMTHREADS; ++i)
    thread_fg_set(&threads[i], THRD_AVAIL_BIT__, true);
}

void
os_run(void)
{
//  if (run_ct) {
//    preempt_init();
//    context_load(&run_ptr->ctx);
//  } else {
//    for (;;);
//  }
  preempt_init();
  preempt_firstrun();
  panic(0);
}

void
os_thread_set(void (*routine1)(void),
              void (*routine2)(void))
{
  thrd_t thr1, thr2;
  thrd_create(&thr1, (thrd_start_t) routine1, NULL);
  thrd_create(&thr2, (thrd_start_t) routine2, NULL);

  thrd_join(thr1, NULL);
  thrd_join(thr2, NULL);
}


void
panic(c)
    int c;
{
  volatile int code = c; // for debug
  for (;;);
}



// Watchdog Timer interrupt service routine
//#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
//#pragma vector=WDT_VECTOR
//__interrupt
//#elif defined(__GNUC__)
//
//__attribute__ ((interrupt(WDT_VECTOR)))
//#else
//#error Compiler not supported!
//#endif
//void
//WDT_ISR(void)
//{
//  context_save(&run_ptr->ctx);
//  schedule();
//  preempt_reset();
//  context_load(&run_ptr->ctx);
//}





