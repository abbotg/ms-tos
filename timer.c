//
// Created by gta on 10/23/19.
//

#include "timer.h"

volatile uint16_t start, stop, isr_time;

void timerA0_init(void)
{
  TA0CTL |= MC_0 | TACLR; // Stop the Timer A0 and clear it.
  TA0CTL |= TASSEL__SMCLK | ID__8 | MC__CONTINUOUS;
}

uint16_t timerA0_val(void)
{
  return TA0R;
}

void isr_time_start(void)
{
  start = TA0R;
}

void isr_time_stop(void)
{
  stop = TA0R;
  isr_time = stop - start;
}


