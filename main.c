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

#include <msp430.h>
#include "os.h"
#include "timer.h"

volatile uint16_t num_mains = 0;


void thread1(void) {
  uint16_t i;
  for (;;) {
    P1OUT ^= BIT0;                      // Toggle P1.0 using exclusive-OR
    __delay_cycles(100000);
//    os_yield();
//    i = 0xFFFF;
//    while (i != 0)
//      i--;
  }
}

void thread2(void) {
  uint16_t i;
  for (;;) {
    P1OUT ^= BIT1;                      // Toggle P1.0 using exclusive-OR

    __delay_cycles(100000);
//    i = 0xFFFF;
//    while (i != 0)
//      i--;
//    os_yield();
  }
}

void main(void)
{
  num_mains++;
  WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;

  // Clock System Setup
  CSCTL0_H = CSKEY_H;                     // Unlock CS registers
  CSCTL1 = DCOFSEL_0;                     // Set DCO to 1MHz
  // Set SMCLK = MCLK = DCO, ACLK = VLOCLK
  CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;

  P1DIR = BIT0 | BIT1;                          // Set P1.0 to output direction

  timerA0_init();

  os_init();
//  os_thread_create(&thread1);
//  os_thread_create(&thread2);
  os_thread_set(&thread1, &thread2);
  os_run();

  for (;;);
}
