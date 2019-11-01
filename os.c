/*
 * os.c
 *
 *  Created on: Oct 18, 2019
 *      Author: gta
 */
#include "os.h"


void
preempt_init(void)
{
//  WDTCTL = WDTPW | WDTSSEL__SMCLK | WDTTMSEL | WDTCNTCL | WDTIS__8192;
  WDTCTL = WDT_ADLY_1_9;
  SFRIE1 |= WDTIE;
}

void
preempt_trigger(void)
{
  SFRIFG1 |= WDTIFG; // wdt interrupt pending
//	IFG1 |= WDTIFG;
}

void
preempt_reset(void)
{
	SFRIFG1 &= ~WDTIFG;
	 WDTCTL = WDT_ADLY_1_9;
}



thread_t
os_thread_create(void (*routine)(void))
{
  int i;
  __disable_interrupt();
  for (i = 0; i < NUMTHREADS; ++i) {
    if (threads[i].available) {
      threads[i].available = false;
      thread_init(threads + i, routine);
      (void) link();
      __enable_interrupt();
      return i;
    }
  }
  __enable_interrupt();
  return -1;
}

void
os_init(void)
{
  __disable_interrupt(); // disable interrupts until os_run
  preempt_reset();
  int i;
  for (i = 0; i < NUMTHREADS; ++i)
    threads[i].available = 0;
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
os_yield(void)
{
  preempt_trigger();
}

void
os_thread_set(void (*routine1)(void),
              void (*routine2)(void))
{

	thread_init(threads + 0, routine1);
	thread_init(threads + 1, routine2);

	threads[0].next = &threads[1];
	threads[1].next = &threads[0];

	run_ptr = &threads[0];
}


void
panic(c)
  int c;
{
  volatile int code = c; // for debug
  for (;;)
    ;
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


