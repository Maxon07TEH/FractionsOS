#include "interrupts.h"
#include "pic.h"
#include "io.h"
void init_interrupts(void){
    pic_remap(32,40);
    enable_interrupts();
}
void enable_interrupts(void){ asm volatile("sti"); }
void disable_interrupts(void){ asm volatile("cli"); }
void register_interrupt_handler(uint8_t irq, void (*h)(void)){
    /* здесь можно сохранять h в свою таблицу */
    (void)irq; (void)h;
}
