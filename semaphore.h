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

int bsem_init(unsigned *sem, unsigned bit, bool value);
int bsem_destroy(unsigned *sem, unsigned bit);
int bsem_wait(unsigned *sem, unsigned bit);
int bsem_trywait(unsigned *sem, unsigned bit);
int bsem_post(unsigned *sem, unsigned bit);
bool bsem_getvalue(unsigned *sem, unsigned bit);



#endif //RTOS_SEMAPHORE_H
