#include "vbe.h"
#include "io.h"
#include "string.h"
#include "memory.h"

vbe_mode_info_t vbe_mode_info;

static const uint8_t default_font[256][16] = {
    /* 0x00 */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* 0x01 */ {0x00,0x00,0x7E,0x81,0xA5,0x81,0x81,0xBD,0x99,0x81,0x81,0x7E,0x00,0x00,0x00,0x00},
    /* ... Полный набор символов VGA 8x16 ... */
};

static void vbe_write(uint16_t index, uint16_t value) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    outw(VBE_DISPI_IOPORT_DATA, value);
}

void vbe_init(uint32_t width, uint32_t height, uint8_t bpp) {
    vbe_write(VBE_DISPI_INDEX_ENABLE, 0);
    vbe_write(VBE_DISPI_INDEX_XRES, width);
    vbe_write(VBE_DISPI_INDEX_YRES, height);
    vbe_write(VBE_DISPI_INDEX_BPP, bpp);
    vbe_write(VBE_DISPI_INDEX_ENABLE, 0x41);

    vbe_mode_info.width = width;
    vbe_mode_info.height = height;
    vbe_mode_info.bpp = bpp;
    vbe_mode_info.pitch = width * 4;
    vbe_mode_info.framebuffer = (uint32_t*)0xE0000000;
    vbe_mode_info.backbuffer = (uint32_t*)AllocateMemory(width * height * 4);
    vbe_mode_info.font_bitmap = (uint32_t*)default_font;
    vbe_mode_info.font_width = 8;
    vbe_mode_info.font_height = 16;
    
    memset(vbe_mode_info.backbuffer, 0, width * height * 4);
}

void vbe_swap_buffers() {
    memcpy(vbe_mode_info.framebuffer, vbe_mode_info.backbuffer, 
          vbe_mode_info.width * vbe_mode_info.height * 4);
}

void vbe_draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if(x >= vbe_mode_info.width || y >= vbe_mode_info.height) return;
    
    uint8_t alpha = (color >> 24) & 0xFF;
    uint32_t bg = vbe_mode_info.backbuffer[y * vbe_mode_info.width + x];
    
    if(alpha == 0xFF) {
        vbe_mode_info.backbuffer[y * vbe_mode_info.width + x] = color;
    } else if(alpha > 0) {
        uint8_t r = ((color >> 16) & 0xFF);
        uint8_t g = ((color >> 8) & 0xFF);
        uint8_t b = (color & 0xFF);
        
        uint8_t bg_r = (bg >> 16) & 0xFF;
        uint8_t bg_g = (bg >> 8) & 0xFF;
        uint8_t bg_b = bg & 0xFF;
        
        r = (r * alpha + bg_r * (255 - alpha)) / 255;
        g = (g * alpha + bg_g * (255 - alpha)) / 255;
        b = (b * alpha + bg_b * (255 - alpha)) / 255;
        
        vbe_mode_info.backbuffer[y * vbe_mode_info.width + x] = RGB(r, g, b);
    }
}

void vbe_draw_bitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint32_t* bitmap) {
    for(uint32_t j = 0; j < height; j++) {
        for(uint32_t i = 0; i < width; i++) {
            uint32_t px = x + i;
            uint32_t py = y + j;
            if(px < vbe_mode_info.width && py < vbe_mode_info.height) {
                vbe_draw_pixel(px, py, bitmap[j * width + i]);
            }
        }
    }
}

void vbe_draw_char(char c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg) {
    const uint8_t* font = &default_font[(uint8_t)c][0];
    for(uint32_t j = 0; j < vbe_mode_info.font_height; j++) {
        for(uint32_t i = 0; i < vbe_mode_info.font_width; i++) {
            uint32_t color = (font[j] & (1 << (7 - i))) ? fg : bg;
            vbe_draw_pixel(x + i, y + j, color);
        }
    }
}

void vbe_draw_string(const char* str, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg) {
    uint32_t cursor_x = x;
    uint32_t cursor_y = y;
    
    while(*str) {
        if(*str == '\n') {
            cursor_x = x;
            cursor_y += vbe_mode_info.font_height;
            str++;
            continue;
        }
        
        vbe_draw_char(*str, cursor_x, cursor_y, fg, bg);
        cursor_x += vbe_mode_info.font_width;
        str++;
    }
}

void vbe_clear(uint32_t color) {
    for(uint32_t i = 0; i < vbe_mode_info.width * vbe_mode_info.height; i++) {
        vbe_mode_info.backbuffer[i] = color;
    }
}

void vbe_scroll(uint32_t lines) {
    uint32_t line_size = vbe_mode_info.width * 4;
    uint32_t scroll_pixels = lines * vbe_mode_info.font_height;
    
    if(scroll_pixels >= vbe_mode_info.height) {
        vbe_clear(0);
        return;
    }
    
    memmove(vbe_mode_info.backbuffer, 
           vbe_mode_info.backbuffer + scroll_pixels * vbe_mode_info.width,
           (vbe_mode_info.height - scroll_pixels) * line_size);
    
    memset(vbe_mode_info.backbuffer + (vbe_mode_info.height - scroll_pixels) * vbe_mode_info.width,
          0, scroll_pixels * line_size);
}

void vbe_set_font(uint32_t* bitmap, uint32_t width, uint32_t height) {
    vbe_mode_info.font_bitmap = bitmap;
    vbe_mode_info.font_width = width;
    vbe_mode_info.font_height = height;
}