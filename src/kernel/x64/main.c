#include "../../drivers/vga.h"

// Атрибут для запрета оптимизации
__attribute__((optimize("O0"))) 
void kernel_main(void) {
    vga_initialize();
    vga_write_string("FractionsOS: OK\n");

    // Бесконечный цикл с CLI и HLT
    __asm__ volatile (
        "cli\n"
        "1: hlt\n"
        "jmp 1b"
    );
}
