//
// Created by gta on 10/23/19.
//

#ifndef RTOS_TIMER_H
#define RTOS_TIMER_H

#include <msp430.h>
#include "defines.h"

inline void timerA0_init(void);
inline uint16_t timerA0_val(void); // units are 8us
inline void isr_time_start(void);
inline void isr_time_stop(void);

#endif // RTOS_TIMER_H
