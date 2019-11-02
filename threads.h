//
// Created by gta on 10/29/19.
//
/*
 * C11 standard (ISO/IEC 9899:2011):
 *    7.26 Threads <threads.h> (p: 376-387)
 *    7.31.15 Threads <threads.h> (p: 456)
 *
 * Documentation in this file is copied from: https://en.cppreference.com/w/c/thread
 *
 * The documentation in this file is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/ or send
 * a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
 */

#ifndef RTOS_THREADS_H
#define RTOS_THREADS_H

#include "os.h"

// alien types
struct timespec;

/*
 * Threads
 */
typedef int (*thrd_start_t)(void *);

enum {
  thrd_success = 0,
  thrd_nomem = 1,
  thrd_timedout = 2,
  thrd_busy = 3,
  thrd_error = 4
};

struct thread;
typedef struct thread thrd_t; // TODO: this should not be a pointer to the actual thread struct

/**
 * Creates a new thread executing the function func. The function is invoked as func(arg).
 * If successful, the object pointed to by thr is set to the identifier of the new thread.
 * The completion of this function synchronizes-with the beginning of the thread.
 * @param thr pointer to memory location to put the identifier of the new thread
 * @param func function to execute
 * @param arg argument to pass to the function
 * @return thrd_success if the creation of the new thread was successful. Otherwise returns thrd_nomem
 * if there was insufficient amount of memory or thrd_error if another error occurred.
 */
int
thrd_create(thrd_t *thr, thrd_start_t func, void *arg);

/**
 * Checks whether lhs and rhs refer to the same thread.
 * @param lhs thread to compare
 * @param rhs thread to compare
 * @return Non-zero value if lhs and rhs refer to the same value, ​0​ otherwise.
 */
int
thrd_equal(thrd_t lhs, thrd_t rhs);

/**
 * Returns the identifier of the calling thread.
 * @return The identifier of the calling thread.
 */
thrd_t
thrd_current(void);

/**
 * Blocks the execution of the current thread for at least until the TIME_UTC based duration pointed
 * to by duration has elapsed.
 * The sleep may resume earlier if a signal that is not ignored is received. In such case,
 * if remaining is not NULL, the remaining time duration is stored into the object pointed to by remaining.
 * @param duration pointer to the duration to sleep for
 * @param remaining  	pointer to the object to put the remaining time on interruption.
 * May be NULL, in which case it is ignored
 * @return 0​on successful sleep, -1 if a signal occurred, other negative value if an error occurred.
 */
int
thrd_sleep(const struct timespec *duration, struct timespec *remaining);

/**
 * Provides a hint to the implementation to reschedule the execution of threads,
 * allowing other threads to run.
 */
void
thrd_yield(void);

/**
 * First, for every thread-specific storage key which was created with a non-null destructor and for which the
 * associated value is non-null (see tss_create), thrd_exit sets the value associated with the key to NULL and
 * then invokes the destructor with the previous value of the key. The order in which the destructors are
 * invoked is unspecified.
 * If, after this, there remain keys with both non-null destructors and values (e.g. if a
 * destructor executed tss_set), the process is repeated up to TSS_DTOR_ITERATIONS times.
 * FInally, the thrd_exit function terminates execution of the calling thread and sets its result code to res.
 * If the last thread in the program is terminated with thrd_exit, the entire program terminates as if
 * by calling exit with EXIT_SUCCESS as the argument (so the functions registered by atexit are
 * executed in the context of that last thread)
 * @param res the result value to return
 */
void
thrd_exit(int res) __attribute__ ((noreturn));

/**
 * Detaches the thread identified by thr from the current environment. The resources held by the thread
 * will be freed automatically once the thread exits.
 * @param thr identifier of the thread to detach
 * @return thrd_success if successful, thrd_error otherwise.
 */
int
thrd_detach(thrd_t thr);

/**
 * Blocks the current thread until the thread identified by thr finishes execution.
 * If res is not a null pointer, the result code of the thread is put to the location pointed to by res.
 * The termination of the thread synchronizes-with the completion of this function.
 * The behavior is undefined if the thread was previously detached or joined by another thread.
 * @param thr identifier of the thread to join
 * @param res location to put the result code to
 * @return thrd_success if successful, thrd_error otherwise.
 */
int
thrd_join(thrd_t thr, int *res);


/*
 * Mutual exclusion
 */
typedef bsem_t mtx_t;
enum {
  mtx_plain,
  mtx_recursive,
  mtx_timed
};

/**
 * Creates a new mutex object with type. The object pointed to by mutex is set to an identifier of the newly created mutex.
 * <tt>type</tt> must have one of the following values:
 * <ul>
 *  <li>mtx_plain - a simple, non-recursive mutex is created.</li>
 *  <li>mtx_timed - a non-recursive mutex, that supports timeout, is created.</li>
 *  <li>mtx_plain | mtx_recursive - a recursive mutex is created.</li>
 *  <li>mtx_timed | mtx_recursive - a recursive mutex, that supports timeout, is created.</li>
 * </ul>
 * @param mutex pointer to the mutex to initialize
 * @param type the type of the mutex
 * @return thrd_success if successful, thrd_error otherwise.
 */
int
mtx_init(mtx_t *mutex, int type);

/**
 * Blocks the current thread until the mutex pointed to by mutex is locked.
 * The behavior is undefined if the current thread has already locked the mutex and the mutex is not recursive.
 * Prior calls to mtx_unlock on the same mutex synchronize-with this operation, and all lock/unlock operations
 * on any given mutex form a single total order (similar to the modification order of an atomic)
 * @param mutex pointer to the mutex to lock
 * @return thrd_success if successful, thrd_error otherwise.
 */
int
mtx_lock(mtx_t *mutex);

/**
 * Blocks the current thread until the mutex pointed to by mutex is locked or until the TIME_UTC based
 * time point pointed to by time_point has been reached.
 * The behavior is undefined if the current thread has already locked the mutex and the mutex is not recursive.
 * The behavior is undefined if the mutex does not support timeout.
 * Prior calls to mtx_unlock on the same mutex synchronize-with this operation (if this operation succeeds),
 * and all lock/unlock operations on any given mutex form a single total order (similar to the
 * modification order of an atomic)
 * @param mutex pointer to the mutex to lock
 * @param time_point pointer to the timeout time to wait until
 * @return thrd_success if successful, thrd_timedout if the timeout time has been reached before the
 * mutex is locked, thrd_error if an error occurs.
 */
int
mtx_timedlock(mtx_t *restrict mutex, const struct timespec *restrict time_point);

/**
 * Tries to lock the mutex pointed to by mutex without blocking. Returns immediately if the mutex is already locked.
 * Prior calls to mtx_unlock on the same mutex synchronize-with this operation (if this operation succeeds),
 * and all lock/unlock operations on any given mutex form a single total order (similar to the modification
 * order of an atomic)
 * @param mutex pointer to the mutex to lock
 * @return thrd_success if successful, thrd_busy if the mutex has already been locked or due to a spurious
 * failure to acquire an available mutex (since C17), thrd_error if an error occurs.
 */
int
mtx_trylock(mtx_t *mutex);

/**
 * Unlocks the mutex pointed to by mutex.
 * The behavior is undefined if the mutex is not locked by the calling thread.
 * This function synchronizes-with subsequent mtx_lock, mtx_trylock, or mtx_timedlock on the
 * same mutex. All lock/unlock operations on any given mutex form a single total order (similar to the
 * modification order of an atomic).
 * @param mutex pointer to the mutex to unlock
 * @return thrd_success if successful, thrd_error otherwise.
 */
int
mtx_unlock(mtx_t *mutex);

/**
 * Destroys the mutex pointed to by mutex.
 * If there are threads waiting on mutex, the behavior is undefined.
 * @param mutex pointer to the mutex to destroy
 */
void
mtx_destroy(mtx_t *mutex);


/*
 * Condition variables
 */
struct condition_variable {
  sem_t private_lock;
  uint8_t num_waiters;
  sem_t threads_to_wakeup;
};
typedef struct condition_variable cnd_t;

/**
 * Initializes new condition variable. The object pointed to by cond will be set
 * to value that identifies the condition variable.
 * @param cond pointer to a variable to store identifier of the condition variable to
 * @return thrd_success if the condition variable was successfully created. Otherwise returns thrd_nomem if
 * there was insufficient amount of memory or thrd_error if another error occurred.
 */
int
cnd_init(cnd_t *cond);

/**
 * Unblocks one thread that currently waits on condition variable pointed to by cond.
 * If no threads are blocked, does nothing and returns thrd_success.
 * @param cond pointer to a condition variable
 * @return thrd_success if successful, thrd_error otherwise.
 */
int
cnd_signal(cnd_t *cond);

/**
 * Unblocks all thread that currently wait on condition variable pointed to by cond.
 * If no threads are blocked, does nothing and returns thrd_success.
 * @param cond pointer to a condition variable
 * @return thrd_success if successful, thrd_error otherwise.
 */
int
cnd_broadcast(cnd_t *cond);

/**
 * Atomically unlocks the mutex pointed to by mutex and blocks on the condition
 * variable pointed to by cond until the thread is signalled by cnd_signal or
 * cnd_broadcast, or until a spurious wake-up occurs. The mutex is locked again
 * before the function returns.
 * The behavior is undefined if the mutex is not already locked by the calling thread.
 * @param cond pointer to the condition variable to block on
 * @param mutex pointer to the condition variable to block on
 * @return thrd_success if successful, thrd_error otherwise.
 */
int
cnd_wait(cnd_t *cond, mtx_t *mutex);

/**
 * Atomically unlocks the mutex pointed to by mutex and blocks on the condition
 * variable pointed to by cond until the thread is signalled by cnd_signal or
 * cnd_broadcast, or until the TIME_UTC based time point pointed to by time_point
 * has been reached, or until a spurious wake-up occurs. The mutex is locked
 * again before the function returns.
 * The behavior is undefined if the mutex is not already locked by the calling thread.
 * @param cond pointer to the condition variable to block on
 * @param mutex pointer to the mutex to unlock for the duration of the block
 * @param time_point pointer to a object specifying timeout time to wait until
 * @return thrd_success if successful, thrd_timedout if the timeout time has been reached before the
 * mutex is locked, or thrd_error if an error occurred.
 */
int
cnd_timedwait(cnd_t *restrict cond, mtx_t *restrict mutex, const struct timespec *restrict time_point);

/**
 * Destroys the condition variable pointed to by cond.
 * If there are threads waiting on cond, the behavior is undefined.
 * @param cond pointer to the condition variable to destroy
 */
void
cnd_destroy(cnd_t *cond);

#endif //RTOS_THREADS_H
