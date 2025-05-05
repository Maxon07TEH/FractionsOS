#ifndef VBE_H
#define VBE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA 0x01CF
#define VBE_DISPI_INDEX_ENABLE 0x4
#define VBE_DISPI_INDEX_XRES 0x1
#define VBE_DISPI_INDEX_YRES 0x2
#define VBE_DISPI_INDEX_BPP 0x3

#define ARGB(a, r, g, b) ((a << 24) | (r << 16) | (g << 8) | b)
#define RGB(r, g, b) ARGB(0xFF, r, g, b)

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t pitch;
    uint32_t* framebuffer;
    uint32_t* backbuffer;
    uint32_t* font_bitmap;
    uint32_t font_width;
    uint32_t font_height;
} vbe_mode_info_t;

void vbe_init(uint32_t width, uint32_t height, uint8_t bpp);
void vbe_swap_buffers();
void vbe_draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void vbe_draw_bitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint32_t* bitmap);
void vbe_draw_char(char c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void vbe_draw_string(const char* str, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);
void vbe_clear(uint32_t color);
void vbe_scroll(uint32_t lines);
void vbe_set_font(uint32_t* bitmap, uint32_t width, uint32_t height);

extern vbe_mode_info_t vbe_mode_info;

#endif