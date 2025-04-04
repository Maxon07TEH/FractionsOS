#include "vga.h"
#include <stdarg.h>

void printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    char buffer[256];
    // Упрощённая реализация форматирования (без поддержки %d, %x и т.д.)
    int i = 0;
    while (*fmt && i < 255) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 's') {
                const char *str = va_arg(args, const char *);
                while (*str) buffer[i++] = *str++;
            }
            fmt++;
        } else {
            buffer[i++] = *fmt++;
        }
    }
    buffer[i] = '\0';
    
    vga_write_string(buffer); // Исправлено на vga_write_string()
    
    va_end(args);
}