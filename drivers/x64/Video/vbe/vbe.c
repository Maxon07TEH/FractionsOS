#include "vbe.h"
#include <stdint.h>
#include <stddef.h>
// #include <stdlib.h>  // malloc не используется – применяем статический буфер
#include "../../../../src/modules/BinFont.h"        // Шрифтовые данные (BinFont.h)
#include "../../../../src/images/headers/placeholder.h"  // placeholder

/* Функция вычисления абсолютного значения */
static inline int my_abs(int x) {
    return (x < 0) ? -x : x;
}

/* Глобальные переменные для работы с фреймбуфером */
static uint32_t *framebuffer;
static uint32_t pitch;         // в байтах
static uint32_t fb_width, fb_height;
static uint32_t fb_bpp;

/* --- Поддержка слоёв --- */
#define MAX_LAYERS 16
// Статически выделяем память для каждого слоя (максимальное разрешение – 1920×1080)
#define MAX_LAYER_PIXELS (1920 * 1080)
static uint32_t layer_storage[MAX_LAYERS][MAX_LAYER_PIXELS];

static uint32_t* layers[MAX_LAYERS] = {0};
static uint8_t layer_enabled[MAX_LAYERS] = {0};

/* Инициализация VBE; параметры получены из multiboot-тега */
void nvidia_vbe_init(uint32_t width, uint32_t height, uint8_t bpp, uint32_t fb_pitch, void *fb_addr) {
    fb_width = width;
    fb_height = height;
    fb_bpp = bpp;
    framebuffer = (uint32_t *)fb_addr;
    pitch = fb_pitch;  // Используем реальное значение pitch (5464 для 1366x768)
}

/* Отрисовка пикселя с учётом pitch */
void draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x < fb_width && y < fb_height) {
        uint32_t *pixel_ptr = (uint32_t *)((uint8_t *)framebuffer + y * pitch);
        pixel_ptr[x] = color;
    }
}

/* Очистка экрана заданным цветом */
void clear_screen(uint32_t color) {
    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            draw_pixel(x, y, color);
        }
    }
}

/* Отрисовка линии алгоритмом Брезенхэма */
void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = my_abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -my_abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

/* Отрисовка прямоугольника заданного цвета */
void draw_rect(uint32_t x, uint32_t y, uint32_t rect_width, uint32_t rect_height, uint32_t color) {
    for (uint32_t j = y; j < y + rect_height && j < fb_height; j++) {
        for (uint32_t i = x; i < x + rect_width && i < fb_width; i++) {
            draw_pixel(i, j, color);
        }
    }
}

/* Отрисовка битмапа изображения (массив uint64_t), напрямую в framebuffer */
void draw_bitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint64_t *bitmap) {
    if (!bitmap) return;
    uint32_t max_x = fb_width;
    uint32_t max_y = fb_height;
    if (x >= max_x || y >= max_y) return;
    for (uint32_t j = 0; j < height; j++) {
        for (uint32_t i = 0; i < width; i++) {
            uint32_t draw_x = x + i;
            uint32_t draw_y = y + j;
            if (draw_x < max_x && draw_y < max_y) {
                uint32_t pixel = (uint32_t)(bitmap[j * width + i] & 0xFFFFFFFF);
                draw_pixel(draw_x, draw_y, pixel);
            }
        }
    }
}

/* Новая функция: отрисовка битмапа изображения в заданном слое */
void draw_bitmap_layer(uint8_t layer_index, uint32_t x, uint32_t y, uint32_t image_width, uint32_t image_height, const uint64_t *bitmap) {
        if (!bitmap) return;
        if (layer_index >= MAX_LAYERS || !layer_enabled[layer_index]) return;
        
        // Количество 64-битных элементов в одной строке изображения
        uint32_t elements_per_row = (image_width + 1) / 2;
        
        for (uint32_t j = 0; j < image_height; j++) {
            for (uint32_t i = 0; i < image_width; i++) {
                uint32_t draw_x = x + i;
                uint32_t draw_y = y + j;
                if (draw_x < fb_width && draw_y < fb_height) {
                    uint32_t element_index = j * elements_per_row + (i / 2);
                    uint64_t element = bitmap[element_index];
                    uint32_t pixel;
                    if ((i & 1) == 0) { // четное: младшие 32 бита
                        pixel = (uint32_t)(element & 0xFFFFFFFF);
                    } else { // нечетное: старшие 32 бита
                        pixel = (uint32_t)((element >> 32) & 0xFFFFFFFF);
                    }
                    layers[layer_index][draw_y * fb_width + draw_x] = pixel;
                }
            }
        }
    }

/* --- Функции поддержки слоёв --- */

/* Создание слоя с индексом layer_index */
void create_layer(uint8_t layer_index) {
    if (layer_index >= MAX_LAYERS) return;
    layers[layer_index] = layer_storage[layer_index];
    layer_enabled[layer_index] = 1;
    for (uint32_t i = 0; i < fb_width * fb_height; i++) {
        layers[layer_index][i] = 0x00000000; // Изначально прозрачный
    }
}

/* Очистка слоя layer_index заданным цветом */
void clear_layer(uint8_t layer_index, uint32_t color) {
    if (layer_index >= MAX_LAYERS || !layer_enabled[layer_index]) return;
    for (uint32_t i = 0; i < fb_width * fb_height; i++) {
        layers[layer_index][i] = color;
    }
}

/* Отрисовка пикселя в слое */
void draw_layer_pixel(uint8_t layer_index, uint32_t x, uint32_t y, uint32_t color) {
    if (layer_index >= MAX_LAYERS || !layer_enabled[layer_index]) return;
    if (x < fb_width && y < fb_height) {
        layers[layer_index][y * fb_width + x] = color;
    }
}

/* Композитинг слоёв: для каждого пикселя берём значение из верхнего слоя, где пиксель непрозрачен */
void composite_layers(void) {
    for (uint32_t y = 0; y < fb_height; y++) {
        for (uint32_t x = 0; x < fb_width; x++) {
            uint32_t final_color = 0xFF000000; // по умолчанию черный, непрозрачный
            for (uint8_t l = 0; l < MAX_LAYERS; l++) {
                if (layer_enabled[l]) {
                    uint32_t pixel = layers[l][y * fb_width + x];
                    if ((pixel >> 24) != 0x00) {
                        final_color = pixel;
                    }
                }
            }
            draw_pixel(x, y, final_color);
        }
    }
}

/* --- Функция отрисовки символа --- */
// Исходный размер шрифта: 12x12 (определён в BinFont.h)
// Минимальный размер для вывода: 16x32 (масштабирование)
#define MIN_FONT_WIDTH  16
#define MIN_FONT_HEIGHT 32

/* Отрисовка символа c на слое layer_index в позиции (x, y) с цветом color.
   Если символ не найден в массиве font, используется placeholder.
   Масштабирование до минимального размера выполняется, а цвет наносится с альфа = 0xAA. */
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
    
    if (layer_index >= MAX_LAYERS || !layer_enabled[layer_index]) return;
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
                        layers[layer_index][draw_y * fb_width + draw_x] = final_color;
                    }
                }
            }
        }
    }
}
