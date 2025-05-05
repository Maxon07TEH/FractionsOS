#ifndef PIC_H
#define PIC_H
#include <stdint.h>
void pic_remap(int off1,int off2);
void pic_send_eoi(uint8_t irq);
#endif
