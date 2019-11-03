//
// Created by gta on 11/2/19.
//

#include "os.h"

////////////////////////////
//// SEMAPHORES LIBRARY ////
////////////////////////////

int
sem_init(sem_t *sem, unsigned int value)
{
  if (!sem || value > SEM_VALUE_MAX) {
    errno = EINVAL;
    return -1;
  }
  __disable_interrupt();
  *sem = value;
  __enable_interrupt();
  return 0;
}

int
sem_destroy(sem_t *sem)
{
  return 0;
}

int
sem_wait(sem_t *sem)
{
  if (!sem) {
    errno = EINVAL;
    return -1;
  }
  __disable_interrupt();
  while (!*sem) {
    __enable_interrupt();
    thrd_yield();
    __disable_interrupt();
  }
  (*sem)--;
  __enable_interrupt();
  return 0;
}

int
sem_trywait(sem_t *sem)
{
  if (!sem) {
    errno = EINVAL;
    return -1;
  }
  __disable_interrupt();
  if (!*sem) {
    errno = EAGAIN;
    __enable_interrupt();
    return -1;
  }
  (*sem)--;
  __enable_interrupt();
  return 0;
}

int
sem_post(sem_t *sem)
{
  if (!sem) {
    errno = EINVAL;
    return -1;
  }
  __disable_interrupt();
  (*sem)++;
  __enable_interrupt();
  return 0;
}

unsigned int
sem_getvalue(sem_t *sem)
{
  return (unsigned int) *sem;
}

int
bsem_init(unsigned *sem, unsigned bit, bool value)
{
  if (!sem || value > 1 || value < 0) {
    errno = EINVAL;
    return -1;
  }
  __disable_interrupt();
  if (value) {
    *sem |= 1u << bit;
  } else {
    *sem &= ~(1u << bit);
  }
  __enable_interrupt();
  return 0;
}

int
bsem_destroy(unsigned *sem, unsigned bit)
{
  return 0;
}

int
bsem_wait(unsigned *sem, unsigned bit)
{
  if (!sem) {
    errno = EINVAL;
    return -1;
  }
  __disable_interrupt();
  while ((*sem & (1u << bit)) == 0) {
    __enable_interrupt();
    thrd_yield();
    __disable_interrupt();
  }
  *sem &= ~(1u << bit);
  __enable_interrupt();
  return 0;
}

int
bsem_trywait(unsigned *sem, unsigned bit)
{
  if (!sem) {
    errno = EINVAL;
    return -1;
  }
  __disable_interrupt();
  if (*sem & (1u << bit)) {
    *sem &= ~(1u << bit);
    __enable_interrupt();
    return 0;
  }
  errno = EAGAIN;
  __enable_interrupt();
  return -1;
}

int
bsem_post(unsigned *sem, unsigned bit)
{
  if (!sem) {
    errno = EINVAL;
    return -1;
  }
  __disable_interrupt();
  *sem &= ~(1u << bit);
  __enable_interrupt();
  return 0;
}

bool bsem_getvalue(unsigned *sem, unsigned bit)
{
  return (bool) *sem & (1u << bit);
}

