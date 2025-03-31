#include "vbe.h"
#include <stdint.h>
#include <stddef.h>
#include "../../../../src/modules/BinFont.h"
#include "../../../../src/images/headers/placeholder.h" // placeholder


uint32_t fb_width, fb_height;
static uint32_t pitch;      
static uint32_t fb_bpp;
static uint32_t *framebuffer;  


static inline int my_abs(int x) {
    return (x < 0) ? -x : x;
}

void nvidia_vbe_init(uint32_t width, uint32_t height, uint8_t bpp, uint32_t fb_pitch, void *fb_addr) {
    fb_width  = width;
    fb_height = height;
    fb_bpp    = bpp;
    framebuffer = (uint32_t *)fb_addr;
    pitch = fb_pitch;
}

void draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x < fb_width && y < fb_height) {
        uint32_t *pixel_ptr = (uint32_t *)((uint8_t *)framebuffer + y * pitch);
        pixel_ptr[x] = color;
    }
}

void clear_screen(uint32_t color) {
    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            draw_pixel(x, y, color);
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = my_abs(x1 - x0), sx = (x0 < x1) ? 1 : -1;
    int dy = -my_abs(y1 - y0), sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_rect(uint32_t x, uint32_t y, uint32_t rect_width, uint32_t rect_height, uint32_t color) {
    for (uint32_t j = y; j < y + rect_height && j < fb_height; j++) {
        for (uint32_t i = x; i < x + rect_width && i < fb_width; i++) {
            draw_pixel(i, j, color);
        }
    }
}

void draw_bitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint64_t *bitmap) {
    if (!bitmap) return;
    for (uint32_t j = 0; j < height; j++) {
        for (uint32_t i = 0; i < width; i++) {
            uint32_t draw_x = x + i;
            uint32_t draw_y = y + j;
            if (draw_x < fb_width && draw_y < fb_height) {
                uint32_t pixel = (uint32_t)(bitmap[j * width + i] & 0xFFFFFFFF);
                pixel |= 0xFF000000;
                draw_pixel(draw_x, draw_y, pixel);
            }
        }
    }
}

#define MAX_LAYER_PIXELS (1920 * 1080)
static uint32_t layer_storage[MAX_LAYERS][MAX_LAYER_PIXELS];
static Layer layers_arr[MAX_LAYERS];
static uint32_t comp_buffer[MAX_LAYER_PIXELS];

void create_layer_ex(uint8_t layer_index, uint32_t virt_width, uint32_t virt_height, uint32_t offset_x, uint32_t offset_y) {
    if (layer_index >= MAX_LAYERS) return;
    layers_arr[layer_index].buffer = layer_storage[layer_index];
    layers_arr[layer_index].virt_width  = virt_width;
    layers_arr[layer_index].virt_height = virt_height;
    layers_arr[layer_index].offset_x    = offset_x;
    layers_arr[layer_index].offset_y    = offset_y;
    layers_arr[layer_index].enabled     = 1;
    for (uint32_t i = 0; i < virt_width * virt_height; i++) {
        layers_arr[layer_index].buffer[i] = 0x00000000; 
    }
}

void clear_layer(uint8_t layer_index, uint32_t color) {
    if (layer_index >= MAX_LAYERS || !layers_arr[layer_index].enabled) return;
    Layer *ly = &layers_arr[layer_index];
    for (uint32_t i = 0; i < ly->virt_width * ly->virt_height; i++) {
        ly->buffer[i] = color;
    }
}

void draw_layer_pixel(uint8_t layer_index, uint32_t x, uint32_t y, uint32_t color) {
    if (layer_index >= MAX_LAYERS || !layers_arr[layer_index].enabled) return;
    Layer *ly = &layers_arr[layer_index];
    if (x < ly->virt_width && y < ly->virt_height) {
        ly->buffer[y * ly->virt_width + x] = color;
    }
}

void draw_bitmap_layer(uint8_t layer_index, uint32_t x, uint32_t y,
                         uint32_t image_width, uint32_t image_height, const uint64_t *bitmap) {
    if (!bitmap) return;
    if (layer_index >= MAX_LAYERS || !layers_arr[layer_index].enabled) return;
    Layer *ly = &layers_arr[layer_index];
    for (uint32_t j = 0; j < image_height; j++) {
        for (uint32_t i = 0; i < image_width; i++) {
            uint32_t draw_x = x + i;
            uint32_t draw_y = y + j;
            if (draw_x < ly->virt_width && draw_y < ly->virt_height) {
                uint32_t pixel = (uint32_t)(bitmap[j * image_width + i] & 0xFFFFFFFF);
                pixel |= 0xFF000000;
                ly->buffer[draw_y * ly->virt_width + draw_x] = pixel;
            }
        }
    }
}

#define MIN_FONT_WIDTH  16
#define MIN_FONT_HEIGHT 32

void draw_symbol(uint8_t layer_index, uint32_t x, uint32_t y, char c, uint32_t color) {
    const uint64_t *bitmap = 0;
    size_t font_count = sizeof(font) / sizeof(FontChar);
    for (size_t i = 0; i < font_count; i++) {
        if (font[i].character == c) {
            bitmap = font[i].bitmap;
            break;
        }
    }
    if (!bitmap) {
        extern const uint64_t placeholder[]; // из placeholder.h
        bitmap = placeholder;
    }
    uint32_t scale_x = (FONT_WIDTH < MIN_FONT_WIDTH) ? (MIN_FONT_WIDTH / FONT_WIDTH) : 1;
    uint32_t scale_y = (FONT_HEIGHT < MIN_FONT_HEIGHT) ? (MIN_FONT_HEIGHT / FONT_HEIGHT) : 1;
    if (layer_index >= MAX_LAYERS || !layers_arr[layer_index].enabled) return;
    Layer *ly = &layers_arr[layer_index];
    for (uint32_t row = 0; row < FONT_HEIGHT; row++) {
        uint64_t row_data = bitmap[row];
        for (uint32_t col = 0; col < FONT_WIDTH; col++) {
            uint8_t bit = (row_data >> (FONT_WIDTH - 1 - col)) & 1;
            if (bit) {
                for (uint32_t sy = 0; sy < scale_y; sy++) {
                    for (uint32_t sx = 0; sx < scale_x; sx++) {
                        uint32_t draw_x = x + col * scale_x + sx;
                        uint32_t draw_y = y + row * scale_y + sy;
                        uint32_t final_color = (0xAA << 24) | (color & 0x00FFFFFF);
                        if (draw_x < ly->virt_width && draw_y < ly->virt_height) {
                            ly->buffer[draw_y * ly->virt_width + draw_x] = final_color;
                        }
                    }
                }
            }
        }
    }
}

void composite_layers(void) {
    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            comp_buffer[y * fb_width + x] = 0xFF000000;
        }
    }
    
    for (uint8_t l = 0; l < MAX_LAYERS; l++) {
        if (!layers_arr[l].enabled) continue;
        Layer *ly = &layers_arr[l];
        for (uint32_t y = 0; y < ly->virt_height; y++) {
            for (uint32_t x = 0; x < ly->virt_width; x++) {
                uint32_t pixel = ly->buffer[y * ly->virt_width + x];
                if ((pixel >> 24) != 0x00) {
                    uint32_t final_x = ly->offset_x + x;
                    uint32_t final_y = ly->offset_y + y;
                    if (final_x < fb_width && final_y < fb_height) {
                        comp_buffer[final_y * fb_width + final_x] = pixel;
                    }
                }
            }
        }
    }
    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            draw_pixel(x, y, comp_buffer[y * fb_width + x]);
        }
    }
}
