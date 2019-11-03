//
// Created by gta on 11/2/19.
//

////////////////////////////
////  THREADS LIBRARY  /////
////////////////////////////

#include "os.h"
#include "threads.h"

int
thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
  int i;
  struct {
    context_t ctx;
    trapframe_t tf;
  } temp_tf;

  __disable_interrupt();
  context_save(&temp_tf);

  if (status == SYS_UNINITIALIZED) {     // Need to create a thread representing main
    if (NUMTHREADS <= 1)
      return thrd_nomem;
    threads[0].ctx = temp_tf.ctx;
    threads[0].tf = temp_tf.tf;
    // TODO: thread member 'stack' is unused here
    thread_fg_set(&threads[0], THRD_AVAIL_BIT__, false);
    bsem_init(&threads[0].flags.raw, THRD_SEM_BIT__, false);
  }

  for (i = 0; i < NUMTHREADS; i++) // find an available thread
    if (thread_fg_get(&threads[i], THRD_AVAIL_BIT__))
      break;
  if (i == NUMTHREADS) {
    __enable_interrupt();
    return thrd_nomem;
  }

  thr = &threads[i];
  thread_fg_set(thr, THRD_AVAIL_BIT__, false);
  bsem_init(&thr->flags.raw, THRD_SEM_BIT__, false);

  // Clears out all memory in the process
  memset(thr, 0, sizeof(thrd_t));

  // Trapframe + variable structure used in every scheduler invocation past the 1st
  // Need location to pop these values off and enter process with clean stack
  word_t starting_sp = (word_t) stack_base(thr) - (sizeof(trapframe_t) + 2 * sizeof(word_t));
  word_t tf_sp = (word_t) stack_base(thr) - sizeof(trapframe_t); // Location of trapframe in the 'dummy' frame loaded on

  thr->ctx.sp = starting_sp; // Initialize the process with the stack preloaded with a trapframe
  *stack_base(thr) = (word_t) &thrd_exit; // set PC on stack, RET will branch here upon function return
  thr->tf = trapframe_init((word_t) func, GIE); // Initialize the trapframe

  // Copy the trapframe over to the stack (will be popped off on boot / every invocation)
  memcpy((void *) tf_sp, &thr->tf, sizeof(trapframe_t));

  thr->ctx.r12 = (word_t) arg; // arg in parameter register

  (void) link();
  __enable_interrupt();
  return thrd_success;
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
  if (bsem_trywait(run_ptr->flags.raw, THRD_SEM_BIT__) == 0) {
    run_ptr->ctx.r12 = (word_t) res;
    bsem_post(&run_ptr->flags.raw, THRD_SEM_BIT__);
  } else {
    panic(9); // thread has exited twice??
  }
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
  if (bsem_wait(&thr.flags.raw, THRD_AVAIL_BIT__) == 0) {
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
  return bsem_init((unsigned *) mutex, 0, true) == 0 ? thrd_success : thrd_error;
}

int
mtx_lock(mtx_t *mutex)
{
  return bsem_wait((unsigned *) mutex, 0) == 0 ? thrd_success : thrd_error;
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
  if (bsem_trywait((unsigned *) mutex, 0) == 0)
    return thrd_success;
  return errno == EAGAIN ? thrd_busy : thrd_error;
}

int
mtx_unlock(mtx_t *mutex)
{
  return bsem_post((unsigned *) mutex, 0) == 0 ? thrd_success : thrd_error;
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


