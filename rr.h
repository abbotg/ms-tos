//
// Created by gta on 10/24/19.
//

#ifndef RTOS_RR_H
#define RTOS_RR_H

#include "scheduler.h"
#include "os.h"

struct thread {
  union {
    struct context ctx;
    word_t regs[10];
  };
  tid_t tid;
  struct thread *next;
  bool available;
};

void schedule(void);
int link(void);

#endif //RTOS_RR_H
