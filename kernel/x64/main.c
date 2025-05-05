#include "../../drivers/x64/Interrupt/io.h"
#include "../../drivers/x64/Interrupt/pic.h"
#include "../../drivers/x64/Interrupt/interrupts.h"
#include "../../drivers/x64/Interrupt/scheduler.h"
#include "../../drivers/x64/Video/vga/vga.h"
#include <stdint.h>

void task1(void* _) { while(1){ vga_write_string("T1\n"); scheduler_yield(); }}
void task2(void* _) { while(1){ vga_write_string("T2\n"); scheduler_yield(); }}

void kernel_main(uint32_t magic, void* mb) {
    vga_init();
    pic_remap(32,40);
    init_interrupts();
    enable_interrupts();

    static uint8_t st1[1024], st2[1024];
    scheduler_create_task(task1,NULL,st1,sizeof(st1),0);
    scheduler_create_task(task2,NULL,st2,sizeof(st2),0);
    scheduler_start();

    while(1) asm volatile("hlt");
}
