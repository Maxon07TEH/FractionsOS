#ifndef VBE_H
#define VBE_H

#include <stdint.h>

void nvidia_vbe_init(uint32_t width, uint32_t height, uint8_t bpp, void *fb_addr);
void draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void clear_screen(uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_rect(uint32_t x, uint32_t y, uint32_t rect_width, uint32_t rect_height, uint32_t color);
void draw_bitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint32_t *bitmap);


#endif
