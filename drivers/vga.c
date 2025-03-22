#include "vga.h"

static uint16_t* vga_buffer = (uint16_t*)0xB8000;
static size_t vga_row = 0;
static size_t vga_column = 0;
static uint8_t vga_color = 0;

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_color = fg | (bg << 4);
}

void vga_initialize(void) {
    vga_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    for(size_t y = 0; y < VGA_HEIGHT; y++) {
        for(size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = (vga_color << 8) | ' ';
        }
    }
}

void vga_write_string(const char* data) {
    while(*data != '\0') {
        if(*data == '\n') {
            vga_column = 0;
            vga_row++;
            data++;
            continue;
        }

        if(vga_column >= VGA_WIDTH) {
            vga_column = 0;
            vga_row++;
        }

        const size_t index = vga_row * VGA_WIDTH + vga_column;
        vga_buffer[index] = (vga_color << 8) | *data;

        vga_column++;
        data++;
    }
}
