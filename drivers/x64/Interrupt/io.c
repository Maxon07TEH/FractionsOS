#include "io.h"
uint8_t inb(uint16_t port){ uint8_t r; asm volatile("inb %1,%0":"=a"(r):"Nd"(port)); return r; }
void    outb(uint16_t port,uint8_t v){ asm volatile("outb %0,%1"::"a"(v),"Nd"(port)); }
uint16_t inw(uint16_t port){ uint16_t r; asm volatile("inw %1,%0":"=a"(r):"Nd"(port)); return r; }
void     outw(uint16_t port,uint16_t v){ asm volatile("outw %0,%1"::"a"(v),"Nd"(port)); }
