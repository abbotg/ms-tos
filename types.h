//
// Created by gta on 11/3/19.
//

/*
 * types.h
 *
 * Types common to all user headers
 */

#ifndef RTOS_TYPES_H
#define RTOS_TYPES_H

#include <msp430.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

/* semaphore.h */
typedef uint8_t sem_t;
typedef uint8_t bsem_t;

/* threads.h */
struct thread;
typedef struct thread thrd_t;
struct condition_variable;
typedef struct condition_variable cnd_t;
typedef int (*thrd_start_t)(void *);
typedef bsem_t mtx_t;

#endif //RTOS_TYPES_H
