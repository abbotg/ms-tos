//
// Created by gta on 10/24/19.
//

#ifndef RTOS_THREAD_H
#define RTOS_THREAD_H

#include "types.h"

typedef uint8_t tid_t;

/* Thread prototype. Threads are defined by the scheduler,
 * but have the generic prorotype:
 * struct thread {
 *   union {
 *     struct context ctx;
 *     word_t regs[9];
 *   };
 *   ...
 * };
 */
struct thread;

struct context {
  word_t r4, r5, r6, r7, r8, r9, r10, sp, sr, pc;
};

word_t thread_get_sp(struct thread *this);
void thread_set_sp(struct thread *this, word_t new);

#endif //RTOS_THREAD_H
