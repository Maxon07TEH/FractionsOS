#ifndef VBE_H
#define VBE_H

#include <stdint.h>

/* Экспортируемые переменные для разрешения экрана */
extern uint32_t fb_width, fb_height;

/* Функции инициализации и отрисовки в реальный framebuffer */
void nvidia_vbe_init(uint32_t width, uint32_t height, uint8_t bpp, uint32_t fb_pitch, void *fb_addr);
void draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void clear_screen(uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_rect(uint32_t x, uint32_t y, uint32_t rect_width, uint32_t rect_height, uint32_t color);
void draw_bitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint64_t *bitmap);

/* --- Поддержка виртуальных экранов (слоёв) --- */
typedef struct {
    uint32_t *buffer;      // Буфер слоя
    uint32_t virt_width;   // Виртуальная ширина слоя
    uint32_t virt_height;  // Виртуальная высота слоя
    uint32_t offset_x;     // Смещение по X при композитинге (на реальном экране)
    uint32_t offset_y;     // Смещение по Y при композитинге
    uint8_t  enabled;      // Флаг включения слоя
} Layer;

#define MAX_LAYERS 16

/* Создание виртуального слоя с заданными параметрами */
void create_layer_ex(uint8_t layer_index, uint32_t virt_width, uint32_t virt_height, uint32_t offset_x, uint32_t offset_y);
/* Очистка слоя заданным цветом */
void clear_layer(uint8_t layer_index, uint32_t color);
/* Отрисовка пикселя в виртуальном слое */
void draw_layer_pixel(uint8_t layer_index, uint32_t x, uint32_t y, uint32_t color);
/* Отрисовка изображения (битмапа) в заданном слое */
void draw_bitmap_layer(uint8_t layer_index, uint32_t x, uint32_t y,
                         uint32_t image_width, uint32_t image_height, const uint64_t *bitmap);
/* Отрисовка символа (с масштабированием и полупрозрачностью) в заданном слое */
void draw_symbol(uint8_t layer_index, uint32_t x, uint32_t y, char c, uint32_t color);
/* Композитинг виртуальных слоёв – итоговое изображение копируется в видеопамять */
void composite_layers(void);

#endif
