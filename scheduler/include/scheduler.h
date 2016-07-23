#ifndef __SCHEDULE__
#define __SCHEDULE__

#include <stdint.h>

#define now                                                 0
#define ms                                                  1

typedef void (*task_handler)(uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3);

void construct_scheduler();
void start_scheduler(void);
void schedule_task(uint32_t when, task_handler handler, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3);

#endif