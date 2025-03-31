#ifndef VBE_H
#define VBE_H

#include <stdint.h>

void nvidia_vbe_init(uint32_t width, uint32_t height, uint8_t bpp, uint32_t fb_pitch, void *fb_addr);
void draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void clear_screen(uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_rect(uint32_t x, uint32_t y, uint32_t rect_width, uint32_t rect_height, uint32_t color);
void draw_bitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint64_t *bitmap);

// Новая функция – отрисовка изображения в указанном слое.
void draw_bitmap_layer(uint8_t layer_index, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint64_t *bitmap);

// Функция отрисовки символа на слое.
// layer_index – индекс слоя (0..15), (x,y) – координаты в пикселях, c – символ, color – цвет (RGB; альфа задается внутри).
void draw_symbol(uint8_t layer_index, uint32_t x, uint32_t y, char c, uint32_t color);

// Функции работы со слоями:
void create_layer(uint8_t layer_index);
void clear_layer(uint8_t layer_index, uint32_t color);
void draw_layer_pixel(uint8_t layer_index, uint32_t x, uint32_t y, uint32_t color);
void composite_layers(void);

#endif
