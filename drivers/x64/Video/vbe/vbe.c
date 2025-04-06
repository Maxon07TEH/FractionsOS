#include "vbe.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#define MAX_LAYERS 16
#define MAX_LAYER_PIXELS (1920 * 1080)

static uint32_t fb_width, fb_height;
static uint32_t pitch;
static uint32_t fb_bpp;
static uint32_t *framebuffer;

// Массивы для хранения 16 слоев
static uint32_t layers[MAX_LAYERS][MAX_LAYER_PIXELS];

void vbe_init(uint32_t width, uint32_t height, uint8_t bpp, uint32_t fb_pitch, void *fb_addr) {
    fb_width  = width;
    fb_height = height;
    fb_bpp    = bpp;
    framebuffer = (uint32_t *)fb_addr;
    pitch = fb_pitch;

    // Инициализация слоев
    for (int i = 0; i < MAX_LAYERS; i++) {
        for (int j = 0; j < MAX_LAYER_PIXELS; j++) {
            layers[i][j] = 0x00000000; // Очистка слоев
        }
    }
}

void draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x < fb_width && y < fb_height) {
        uint32_t *pixel_ptr = (uint32_t *)((uint8_t *)framebuffer + y * pitch + x * (fb_bpp / 8));
        *pixel_ptr = color; // Устанавливаем цвет пикселя
    }
}

void draw_bitmap(uint32_t layer_index, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint64_t *bitmap) {
    if (layer_index >= MAX_LAYERS || !bitmap) return;
    for (uint32_t j = 0; j < height; j++) {
        for (uint32_t i = 0; i < width; i++) {
            uint32_t draw_x = x + i;
            uint32_t draw_y = y + j;
            if (draw_x < fb_width && draw_y < fb_height) {
                uint32_t pixel = (uint32_t)(bitmap[j * width + i] & 0xFFFFFFFF);
                layers[layer_index][draw_y * fb_width + draw_x] = pixel; // Сохранение в слой
            }
        }
    }
}

void composite_layers(void) {
    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            uint32_t final_pixel = 0x00000000; // Начинаем с полностью прозрачного
            uint8_t final_alpha = 0;

            for (uint8_t l = 0; l < MAX_LAYERS; l++) {
                uint32_t pixel = layers[l][y * fb_width + x];
                uint8_t alpha = pixel >> 24; // Получаем альфа-канал

                if (alpha > 0) { // Если пиксель не полностью прозрачный
                    uint32_t color = pixel & 0x00FFFFFF; // Получаем цвет без альфа-канала

                    // Смешивание цветов
                    if (final_alpha == 0) {
                        final_pixel = pixel; // Если финальный пиксель еще не установлен
                        final_alpha = alpha;
                    } else {
                        final_alpha = final_alpha + alpha * (255 - final_alpha) / 255; // Итоговый альфа
                        final_pixel = ((final_pixel & 0x00FFFFFF) * final_alpha + (color * alpha * (255 - final_alpha) / 255)) / final_alpha;
                    }
                }
            }

            // Устанавливаем финальный пиксель
            draw_pixel(x, y, (final_alpha << 24) | final_pixel);
        }
    }
}

void layer_clear(uint8_t layer_index) {
    if (layer_index >= MAX_LAYERS) return;
    for (uint32_t i = 0; i < MAX_LAYER_PIXELS; i++) {
        layers[layer_index][i] = 0x00000000; // Очистка слоя
    }
}
