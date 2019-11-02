//
// Created by gta on 10/24/19.
//

#ifndef RTOS_SCHEDULER_H
#define RTOS_SCHEDULER_H

#include "config.h"
#include "thread.h"

thread *run_ptr; // currently running thread

// scheduler contract functions
// all schedulers implement these functions
void schedule(void);
void sched_add(struct thread *);
void sched_start(void);



#if SCHEDULER_USE_ROUND_ROBIN
#include "rr.h"
#elif SCHEDULER_USE_PRIORITY
#include ""
#else
#error "No scheduler defined"
#endif

#endif //RTOS_SCHEDULER_H
