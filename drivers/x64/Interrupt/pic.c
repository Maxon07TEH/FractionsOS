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
