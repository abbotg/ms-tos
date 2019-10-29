//
// Created by gta on 10/29/19.
//

#ifndef RTOS_SEMAPHORE_H
#define RTOS_SEMAPHORE_H

struct semaphore;
typedef struct semaphore sem_t;

int sem_init(sem_t *sem, unsigned int value);

int sem_destroy(sem_t *sem);

int sem_wait(sem_t *sem);

int sem_trywait(sem_t *sem);

int sem_post(sem_t *sem);

unsigned int sem_getvalue(sem_t *sem);

#endif //RTOS_SEMAPHORE_H
