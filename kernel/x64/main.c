#include "../../drivers/vga.h"

void kernel_main(void) {
    vga_initialize();
    vga_write_string("FractionsOS v0.1\n");
    vga_write_string("System initialized successfully!\n");

    // Явный бесконечный цикл с CLI
    __asm__ volatile (
        "cli\n"
        "1: hlt\n"
        "jmp 1b"
    );
}
