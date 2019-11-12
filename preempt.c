//
// Created by gta on 11/3/19.
//

#include "os.h"

void
preempt_init(void)
{
//  WDTCTL = WDTPW | WDTSSEL__SMCLK | WDTTMSEL | WDTCNTCL | WDTIS__8192;
	UCSCTL5 = DIVA_1;
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
