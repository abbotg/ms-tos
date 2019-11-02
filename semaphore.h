//
// Created by gta on 10/29/19.
//

#ifndef RTOS_SEMAPHORE_H
#define RTOS_SEMAPHORE_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_NUM_SEMAPHORES 255
#define SEM_VALUE_MAX 255

typedef uint8_t sem_t;
typedef uint8_t bsem_t;

int sem_init(sem_t *sem, unsigned int value);
int sem_destroy(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_post(sem_t *sem);
unsigned int sem_getvalue(sem_t *sem);

int bsem_init(bsem_t *sem, bool value);
int bsem_destroy(bsem_t *sem);
int bsem_wait(bsem_t *sem);
int bsem_trywait(bsem_t *sem);
int bsem_post(bsem_t *sem);
bool bsem_getvalue(bsem_t *sem);



#endif //RTOS_SEMAPHORE_H
