#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <stdint.h>
typedef void (*task_func_t)(void*);
uint32_t scheduler_create_task(task_func_t f, void* arg, uint8_t* stack, uint32_t sz, uint8_t prio);
void scheduler_start(void);
void scheduler_yield(void);
void scheduler_set_priority(uint32_t pid,uint8_t p);
#endif
