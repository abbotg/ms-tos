//
// Created by gta on 11/2/19.
//

////////////////////////////
////  THREADS LIBRARY  /////
////////////////////////////

#include "threads.h"

int
thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
  int i;
  context_t ctx;


  if (status == SYS_UNINITIALIZED) {

  }
  __disable_interrupt();
  for (i = 0; i < NUMTHREADS; ++i) {
    if (threads[i].available) {
      thr = &threads[i];
      thr->available = false;
      // Clears out all memory in the process
      memset(thr, 0, sizeof(thrd_t));

      // Trapframe + variable structure used in every scheduler invocation past the 1st
      // Need location to pop these values off and enter process with clean stack
      word_t starting_sp = (word_t) stack_base(thr) - (sizeof(trapframe_t) + sizeof(word_t));
      word_t tf_sp =
          (word_t) stack_base(thr) - sizeof(trapframe_t); // Location of trapframe in the 'dummy' frame loaded on
      thr->tf = trapframe_init((word_t) func, GIE); // Initialize the trapframe

      // Initialize the process with the stack preloaded with a trapframe
      thr->ctx.sp = starting_sp;

      // Copy the trapframe over to the stack (will be popped off on boot / every invocation)
      memcpy((void *) tf_sp, &thr->tf, sizeof(trapframe_t));

      thr->ctx.r12 = (word_t) arg; // arg in parameter register
      (void) link();
      __enable_interrupt();
      return thrd_success;
    }
  }
  __enable_interrupt();
  return thrd_nomem;
}


int
thrd_equal(thrd_t lhs, thrd_t rhs)
{
  return lhs.tid == rhs.tid;
}

thrd_t
thrd_current(void)
{
  return (thrd_t) *run_ptr;
}

int
thrd_sleep(const struct timespec *duration, struct timespec *remaining)
{
  return 0;
}

void
thrd_yield(void)
{
  preempt_trigger();
}

void
thrd_exit(int res)
{
  run_ptr->ctx.r12 = (word_t) res;
  bsem_post(&run_ptr->finish_sem);
}

int
thrd_detach(thrd_t thr)
{
  // TODO: implement this
  return 0;
}

int // TODO: thrd_t should not refer to actual struct thread for pass-by-value use cases like below
thrd_join(thrd_t thr, int *res)
{
  if (bsem_wait(&thr.finish_sem) == 0) {
    if (res)
      *res = thr.ctx.r12;
    return thrd_success;
  }
  return thrd_error;
}

////////////////////////////
////   MUTEX LIBRARY   /////
////////////////////////////

/*
 * TODO: type parameter currently is ignored
 */
int
mtx_init(mtx_t *mutex, int type)
{
  return bsem_init((bsem_t *) mutex, true) == 0 ? thrd_success : thrd_error;
}

int
mtx_lock(mtx_t *mutex)
{
  return bsem_wait((sem_t *) mutex) == 0 ? thrd_success : thrd_error;
}

int
mtx_timedlock(mtx_t *mutex, const struct timespec *time_point)
{
  // TODO: implement this
  return 0;
}

int
mtx_trylock(mtx_t *mutex)
{
  if (bsem_trywait((sem_t *) mutex) == 0)
    return thrd_success;
  return errno == EAGAIN ? thrd_busy : thrd_error;
}

int
mtx_unlock(mtx_t *mutex)
{
  return bsem_post((sem_t *) mutex) == 0 ? thrd_success : thrd_error;
}

void
mtx_destroy(mtx_t *mutex)
{
  // TODO: implement this
}

/////////////////////////////////////
//// CONDITION VARIABLES LIBRARY ////
/////////////////////////////////////

int
cnd_init(cnd_t *cond)
{
  cond->num_waiters = 0;
  return sem_init(&cond->private_lock, 1) == 0
         && sem_init(&cond->threads_to_wakeup, 0) == 0
         ? thrd_success : thrd_error;
}

int
cnd_signal(cnd_t *cond)
{
  if (sem_wait(&cond->private_lock) < 0)
    return thrd_error;
  if (cond->num_waiters > 0) {
    if (sem_post(&cond->threads_to_wakeup) < 0)
      return thrd_error;
    (cond->num_waiters)--;
  }
  if (sem_post(&cond->private_lock) < 0)
    return thrd_error;
  return thrd_success;
}

int
cnd_broadcast(cnd_t *cond)
{
  if (sem_wait(&cond->private_lock) < 0)
    return thrd_error;
  while (cond->num_waiters > 0) {
    if (sem_post(&cond->threads_to_wakeup) > 0)
      return thrd_error;
    (cond->num_waiters)--;
  }
  if (sem_post(&cond->private_lock) < 0)
    return thrd_error;
  return 0;
}

int
cnd_wait(cnd_t *cond, mtx_t *mutex)
{
  if (sem_wait(&cond->private_lock) < 0)
    return thrd_error;
  (cond->num_waiters)++;
  if (sem_post(&cond->private_lock) == 0
      && mtx_unlock(mutex) == thrd_success
      && sem_wait(&cond->threads_to_wakeup) == 0
      && mtx_lock(mutex) == thrd_success)
    return thrd_success;
  return thrd_error;
}

int
cnd_timedwait(cnd_t *cond, mtx_t *mutex, const struct timespec *time_point)
{
  // TODO: implement
  return 0;
}

void
cnd_destroy(cnd_t *cond)
{
  // TODO: implement
}


