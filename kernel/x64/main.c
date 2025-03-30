#include "../../drivers/x64/Video/vbe/vbe.h"
#include "../../drivers/x64/Video/vga/vga.h"
#include "../../src/images/headers/wallpaper1.h"
#include "../../src/images/headers/commandprompt.h"
#include "../../src/modules/BinFont.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#define VideoMode 1 // 0 - VGA, 1 - VBE

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
    uint32_t framebuffer_pitch;   // Добавляем поле pitch
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;     // 1 = RGB
    uint16_t reserved;            // Для выравнивания, если требуется
} __attribute__((packed));

void kernel_main(uint32_t magic, void *mboot_info_ptr) {
    if (magic != 0x36d76289) {
        vga_init();
        vga_write_string("[Eroor] Wrong multuboot2 magic.\n");
        while (1) {
            asm volatile("hlt");
        }
    }
    
    // важно капец
    struct multiboot_tag *tag = (struct multiboot_tag *)((uintptr_t)mboot_info_ptr + 8);
    struct multiboot_tag_framebuffer *fb_tag = NULL;

    while (tag->type != 0) {
        if (tag->type == 8) { // MULTIBOOT_TAG_TYPE_FRAMEBUFFER
            fb_tag = (struct multiboot_tag_framebuffer *)tag;
            if (fb_tag->framebuffer_type == 1) { // RGB
                break;
            }
        }
        tag = (struct multiboot_tag *)((uintptr_t)tag + ((tag->size + 7) & ~7));
    }

    if (fb_tag == NULL) {
        // Нет подходящего режима фреймбуфера, переходим в режим VGA
        vga_init();
        vga_write_string("[Error] Wrong framebuffer. stopping code.\n");
        while (1) {
            asm volatile("hlt");
        }
    }

    nvidia_vbe_init(
        fb_tag->framebuffer_width,
        fb_tag->framebuffer_height,
        fb_tag->framebuffer_bpp,
        fb_tag->framebuffer_pitch, 
        (void*)(uintptr_t)fb_tag->framebuffer_addr
    );
    

    // Очищаем экран (черный фон)
    //clear_screen(0xFF000000); // ARGB: непрозрачный черный
    
    //draw_bitmap(50, 50, 200, 200, logoBW);
    draw_bitmap(0, 0, 1366, 768, wallpaper1);
    draw_symbol(0, 0, 'A'); // Вместо "A" используем 'A'


    while (1) {
        asm volatile("hlt");
    }
}
