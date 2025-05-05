#ifndef INTERRUPTS_H
#define INTERRUPTS_H
#include <stdint.h>
void init_interrupts(void);
void enable_interrupts(void);
void disable_interrupts(void);
void register_interrupt_handler(uint8_t irq, void (*h)(void));
#endif
