#include "../../drivers/x64/Video/vbe/vbe.h"
#include "../../src/images/headers/wallpaper1.h" // Определяет WALLPAPER1_WIDTH, WALLPAPER1_HEIGHT, wallpaper1
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot_tag_framebuffer {
    uint32_t type;                // 8 (MULTIBOOT_TAG_TYPE_FRAMEBUFFER)
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint32_t framebuffer_pitch;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;     // 1 = RGB
    uint16_t reserved;            
} __attribute__((packed));

void kernel_main(uint32_t magic, void *mboot_info_ptr) {
    if (magic != 0x36d76289) {
        // Обработка ошибки
        while (1) { asm volatile("hlt"); }
    }
    
    struct multiboot_tag *tag = (struct multiboot_tag *)((uintptr_t)mboot_info_ptr + 8);
    struct multiboot_tag_framebuffer *fb_tag = NULL;
    while (tag->type != 0) {
        if (tag->type == 8) { // FRAMEBUFFER
            fb_tag = (struct multiboot_tag_framebuffer *)tag;
            if (fb_tag->framebuffer_type == 1) { break; }
        }
        tag = (struct multiboot_tag *)((uintptr_t)tag + ((tag->size + 7) & ~7));
    }
    
    if (!fb_tag) {
        // Обработка ошибки
        while (1) { asm volatile("hlt"); }
    }
    
    vbe_init(
        fb_tag->framebuffer_width,
        fb_tag->framebuffer_height,
        fb_tag->framebuffer_bpp,
        fb_tag->framebuffer_pitch,
        (void*)(uintptr_t)fb_tag->framebuffer_addr
    );    
    
    // Создаем 2 слоя
    layer_clear(0);
    layer_clear(1);
    
    // Рисуем обои на слое 0
    draw_bitmap(0, 0, 0, WALLPAPER1_WIDTH, WALLPAPER1_HEIGHT, wallpaper1);
    
    // Очищаем слой 1 и рисуем символ 'A'
    layer_clear(1);
    // Здесь вы можете добавить код для рисования символа 'A' на слое 1, если у вас есть соответствующий bitmap
    
    // Композитим виртуальные слои и выводим итоговое изображение в видеопамять
    composite_layers();
    
    while (1) { asm volatile("hlt"); }
}
