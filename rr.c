//
// Created by gta on 10/24/19.
//

#include "os.h"

static int link(void);

struct thread_node {
  struct thread *this, *next;
};

struct thread threads[NUMTHREADS];

void
schedule(void)
{
  run_ptr =  run_ptr->next;
}

struct thread *
sched_new(void)
{
  int i;
  for (i = 0; i < NUMTHREADS; i++) // find an available thread
    if (thread_fg_get(&threads[i], THRD_AVAIL_BIT__))
      break;
  if (i == NUMTHREADS) {
    __enable_interrupt();
    return NULL;
  }
  return &threads[i];
}

void
sched_add(struct thread *thr)
{
  (void) link();
}

void
sched_start(void)
{
  if (status == SYS_UNINITIALIZED) {
    status = SYS_RUNNING;
    __enable_interrupt();
    preempt_init();
    preempt_firstrun();
    panic(0);
  }
}

void
sched_init(void) {
  int i;
  __disable_interrupt(); // disable interrupts until os_run
  preempt_reset();
  for (i = 0; i < NUMTHREADS; ++i)
    thread_fg_set(&threads[i], THRD_AVAIL_BIT__, true);
}

void
sched_thread_exit(void)
{
  thread_fg_set(run_ptr, THRD_AVAIL_BIT__, true);
  (void) link();
  schedule(); // TODO: Return value from thread needs to be saved somewhere other than context, b/c it could be overwritten
  preempt_firstrun();
  __enable_interrupt();
}

int
link(void)
{
  int i;
  int ct = 0;
  struct thread *prev = 0, *first = 0;
  if (NUMTHREADS == 0)
    return ct;
  for (i = 0; i < NUMTHREADS; i++) {
    if (!thread_fg_get(&threads[i], THRD_AVAIL_BIT__)) {
      ct++;
      if (!first)              // get reference to first to wrap around at the end
        first = &threads[i];
      if (prev)                // only set previous's next if previous has been set
        prev->next = &threads[i]; // (accounts for first iteration of the loop)
      prev = &threads[i];
    }
  }
  prev->next = first;
  run_ptr = first;
  return ct;
}
