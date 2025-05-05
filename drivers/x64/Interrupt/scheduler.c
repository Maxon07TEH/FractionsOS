#include "scheduler.h"
#include "pic.h"
#include "interrupts.h"
#include "io.h"
#include <stdint.h>
#define MAX_TASKS 32
#define STACK_ALIGN 16

typedef struct{uint8_t* sp;} TCB;
static TCB tasks[MAX_TASKS];
static uint32_t current=0, total=0;

static void schedule(void){
    tasks[current].sp=(uint8_t*)__builtin_frame_address(0);
    current=(current+1)%total;
    asm volatile("movl %0,%%esp"::"m"(tasks[current].sp));
}

static void timer_handler(void){
    pic_send_eoi(0);
    schedule();
}

uint32_t scheduler_create_task(task_func_t f,void* arg,uint8_t* st,uint32_t sz,uint8_t prio){
    (void)prio;
    if(total>=MAX_TASKS) return -1;
    uintptr_t sp=((uintptr_t)st+sz)&~(STACK_ALIGN-1);
    uint32_t* s=(uint32_t*)sp;
    *--s=(uint32_t)arg;
    *--s=0;
    *--s=(uint32_t)f;
    tasks[total++].sp=(uint8_t*)s;
    return total-1;
}

void scheduler_start(void){
    register_interrupt_handler(32,timer_handler);
    init_interrupts();
    schedule();
}

void scheduler_yield(void){ schedule(); }
void scheduler_set_priority(uint32_t pid,uint8_t p){ (void)pid;(void)p; }
