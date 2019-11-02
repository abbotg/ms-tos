//
// Created by gta on 10/24/19.
//

#include "rr.h"

void
schedule(void)
{
  run_ptr =  run_ptr->next;
}

int
link(void)
{
  int i;
  int ct = 0;
  thread *prev = 0, *first = 0;
  if (NUMTHREADS == 0)
    return ct;
  for (i = 0; i < NUMTHREADS; i++) {
    if (!threads[i].available) {
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
