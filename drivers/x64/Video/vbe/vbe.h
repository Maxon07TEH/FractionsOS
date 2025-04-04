#ifndef VBE_H
#define VBE_H

#include <stdint.h>

#define MAX_LAYERS 16
#define MAX_LAYER_PIXELS (1920 * 1080)

void vbe_init(uint32_t width, uint32_t height, uint8_t bpp, uint32_t fb_pitch, void *fb_addr);
void draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void draw_bitmap(uint32_t layer_index, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint64_t *bitmap);
void composite_layers(void);
void layer_clear(uint8_t layer_index);

#endif // VBE_H
