#include "vbe.h"
#include <stdint.h>

/* Реализация собственной функции для вычисления абсолютного значения */
static inline int my_abs(int x) {
    return (x < 0) ? -x : x;
}

static uint32_t *framebuffer;
static uint32_t pitch;
static uint32_t fb_width, fb_height;
static uint32_t fb_bpp;

void nvidia_vbe_init(uint32_t width, uint32_t height, uint8_t bpp, uint32_t fb_pitch, void *fb_addr) {
    fb_width = width;
    fb_height = height;
    fb_bpp = bpp;
    framebuffer = (uint32_t *)fb_addr;
    pitch = fb_pitch;  // pitch в байтах, полученный из multiboot тега
}


void draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x < fb_width && y < fb_height) {
        uint32_t line_pixels = fb_width / 4;  // для 32 бит: pitch / 4
        framebuffer[y * line_pixels + x] = color;
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
    int dx = my_abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -my_abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void draw_rect(uint32_t x, uint32_t y, uint32_t rect_width, uint32_t rect_height, uint32_t color) {
    for (uint32_t j = y; j < y + rect_height && j < fb_height; j++) {
        for (uint32_t i = x; i < x + rect_width && i < fb_width; i++) {
            draw_pixel(i, j, color);
        }
    }
}


void draw_bitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint32_t *bitmap) {
    if (!framebuffer || !bitmap) return; // Проверка, что буфер и данные существуют

    uint32_t max_x = pitch / 4;
    uint32_t max_y = 0x1000000 / pitch;

    if (x >= max_x || y >= max_y) return; // Если начальные координаты за границей экрана, выходим

    for (uint32_t j = 0; j < height; j++) {
        for (uint32_t i = 0; i < width; i++) {
            uint32_t draw_x = x + i;
            uint32_t draw_y = y + j;

            if (draw_x < max_x && draw_y < max_y) {
                uint32_t color = bitmap[j * width + i]; // Получаем цвет из битмапа
                draw_pixel(draw_x, draw_y, color); // Используем алгоритм отрисовки пикселя из квадрата
            }
        }
    }
}
/*


void draw_bitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint32_t *bitmap) {
    if (!framebuffer || !bitmap) return;
    
    for (uint32_t j = 0; j < height; j++) {
        uint32_t draw_y = y + j;
        if (draw_y >= fb_height) break;
        
        for (uint32_t i = 0; i < width; i++) {
            uint32_t draw_x = x + i;
            if (draw_x >= fb_width) break;
            
            uint32_t color = bitmap[j * width + i];
            draw_pixel(draw_x, draw_y, color);
        }
    }
}
*/