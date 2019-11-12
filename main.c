//***************************************************************************************
//  MSP430 Blink the LED Demo - Software Toggle P1.0
//
//  Description; Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430x5xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|-->LED
//
//  Texas Instruments, Inc
//  July 2013
//***************************************************************************************

#ifndef RTOS_MAIN_
#define RTOS_MAIN_

#include <msp430.h>
#include "os.h"
#include "timer.h"


volatile int f0 = 0;
volatile int f1 = 1;

int fib(int x, int y, int z, int w) {
	return x + y + z + w;
}

int thread1(void *arg) {
	volatile int*k = ((volatile int*)arg);
  for (;;) {
    P1OUT ^= BIT0;                      // Toggle P1.0 using exclusive-OR
    *k = *k + 1;

    __delay_cycles(100000);



//    os_yield();
//    i = 0xFFFF;
//    while (i != 0)
//      i--;
  }
}

int thread2(void *arg) {
	volatile int*k = ((volatile int*)arg);
  for (;;) {
   P4OUT ^= BIT7;                      // Toggle P4.7 using exclusive-OR
   *k = *k + 1;
    __delay_cycles(100000);
//    i = 0xFFFF;
//    while (i != 0)
//      i--;
//    os_yield();
  }
}

void main(void)
{
  WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

//  // Disable the GPIO power-on default high-impedance mode to activate
//  // previously configured port settings
//  PM5CTL0 &= ~LOCKLPM5;
//
//  // Clock System Setup
//  CSCTL0_H = CSKEY_H;                     // Unlock CS registers
//  CSCTL1 = DCOFSEL_0;                     // Set DCO to 1MHz
//  // Set SMCLK = MCLK = DCO, ACLK = VLOCLK
//  CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;

  P1DIR = BIT0 | BIT1;                          // Set P1.0 to output direction
  P4DIR = BIT7;

//  volatile trapframe_t t = trapframe((word_t) os_run, GIE);

//  asm volatile ("   reti\n");

  timerA0_init();

  os_init();
//  os_thread_create(&thread1);
//  os_thread_create(&thread2);
  os_thread_set(&thread1, &thread2);
  os_run();

  for (;;);
}

//#pragma vector = WDT_VECTOR
//interrupt void __attribute__((naked)) callback(void) {
////	context_switcher();
//}

#endif
