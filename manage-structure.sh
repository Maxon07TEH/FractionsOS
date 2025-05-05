#!/bin/bash
set -e

echo "=== 1. Создаём папку и файлы драйвера прерываний ==="
mkdir -p drivers/x64/Interrupt

# io.h / io.c
cat > drivers/x64/Interrupt/io.h << 'EOF'
#ifndef IO_H
#define IO_H
#include <stdint.h>
uint8_t  inb(uint16_t port);
void     outb(uint16_t port, uint8_t val);
uint16_t inw(uint16_t port);
void     outw(uint16_t port, uint16_t val);
#endif
EOF

cat > drivers/x64/Interrupt/io.c << 'EOF'
#include "io.h"
uint8_t inb(uint16_t port){ uint8_t r; asm volatile("inb %1,%0":"=a"(r):"Nd"(port)); return r; }
void    outb(uint16_t port,uint8_t v){ asm volatile("outb %0,%1"::"a"(v),"Nd"(port)); }
uint16_t inw(uint16_t port){ uint16_t r; asm volatile("inw %1,%0":"=a"(r):"Nd"(port)); return r; }
void     outw(uint16_t port,uint16_t v){ asm volatile("outw %0,%1"::"a"(v),"Nd"(port)); }
EOF

# pic.h / pic.c
cat > drivers/x64/Interrupt/pic.h << 'EOF'
#ifndef PIC_H
#define PIC_H
#include <stdint.h>
void pic_remap(int off1,int off2);
void pic_send_eoi(uint8_t irq);
#endif
EOF

cat > drivers/x64/Interrupt/pic.c << 'EOF'
#include "pic.h"
#include "io.h"
#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1
void pic_remap(int o1,int o2){
    uint8_t m=inb(PIC1_DATA), s=inb(PIC2_DATA);
    outb(PIC1_CMD,0x11); outb(PIC2_CMD,0x11);
    outb(PIC1_DATA,o1);  outb(PIC2_DATA,o2);
    outb(PIC1_DATA,0x04);outb(PIC2_DATA,0x02);
    outb(PIC1_DATA,0x01);outb(PIC2_DATA,0x01);
    outb(PIC1_DATA,m);   outb(PIC2_DATA,s);
}
void pic_send_eoi(uint8_t irq){
    if(irq>=8) outb(PIC2_CMD,0x20);
    outb(PIC1_CMD,0x20);
}
EOF

# interrupts.h / interrupts.c
cat > drivers/x64/Interrupt/interrupts.h << 'EOF'
#ifndef INTERRUPTS_H
#define INTERRUPTS_H
#include <stdint.h>
void init_interrupts(void);
void enable_interrupts(void);
void disable_interrupts(void);
void register_interrupt_handler(uint8_t irq, void (*h)(void));
#endif
EOF

cat > drivers/x64/Interrupt/interrupts.c << 'EOF'
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
EOF

# isr_stubs.S
cat > drivers/x64/Interrupt/isr_stubs.S << 'EOF'
.global isr_stub_table
.type   isr_stub_table,@object
isr_stub_table:
    .rept 256
        .quad isr_common_stub
    .endr

.global isr_common_stub
.type   isr_common_stub,@function
isr_common_stub:
    pusha
    mov $0x10,%ax
    mov %ax,%ds
    mov %ax,%es
    push %eax
    push %eax
    call isr_handler_dispatch
    add $8,%esp
    popa
    iret
EOF

# scheduler.h / scheduler.c
cat > drivers/x64/Interrupt/scheduler.h << 'EOF'
#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <stdint.h>
typedef void (*task_func_t)(void*);
uint32_t scheduler_create_task(task_func_t f, void* arg, uint8_t* stack, uint32_t sz, uint8_t prio);
void scheduler_start(void);
void scheduler_yield(void);
void scheduler_set_priority(uint32_t pid,uint8_t p);
#endif
EOF

cat > drivers/x64/Interrupt/scheduler.c << 'EOF'
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
EOF

echo "=== 2. Перезаписываем scripts/compile.sh ==="
cat > scripts/compile.sh << 'EOF'
#!/bin/bash
ARCH=${1#-}
OBJ="build/obj"
BIN="build/bin"
mkdir -p kernel/$ARCH/$OBJ $OBJ $BIN

nasm -f elf32 -g -F dwarf -o kernel/$ARCH/$OBJ/boot.o kernel/$ARCH/boot.asm
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -I./drivers -O2 -Wall -c kernel/$ARCH/main.c -o kernel/$ARCH/$OBJ/main.o
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -c drivers/x64/Video/vga/vga.c        -o $OBJ/vga.o
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -c drivers/x64/Interrupt/io.c        -o $OBJ/io.o
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -c drivers/x64/Interrupt/pic.c       -o $OBJ/pic.o
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -c drivers/x64/Interrupt/interrupts.c -o $OBJ/interrupts.o
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -c drivers/x64/Interrupt/scheduler.c  -o $OBJ/scheduler.o
nasm -f elf32 drivers/x64/Interrupt/isr_stubs.S -o $OBJ/isr_stubs.o

ld -m elf_i386 -T linker.ld -o $BIN/FractionsOS.$ARCH.bin \
    kernel/$ARCH/$OBJ/boot.o \
    kernel/$ARCH/$OBJ/main.o \
    $OBJ/vga.o $OBJ/io.o $OBJ/pic.o $OBJ/interrupts.o $OBJ/scheduler.o $OBJ/isr_stubs.o
EOF
chmod +x scripts/compile.sh

echo "=== 3. Правим kernel/x64/main.c ==="
cat > kernel/x64/main.c << 'EOF'
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
EOF

echo "=== 4. Собираем и запускаем ==="
make clean && make all && make iso && make run
