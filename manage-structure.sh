#!/bin/bash

# Обновляем main.c
cat > src/kernel/x64/main.c << 'EOF'
#include "../../drivers/x64/Video/vbe/vbe.h"
#include <stdint.h>

struct multiboot_tag_framebuffer {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint32_t framebuffer_pitch;
    uint8_t framebuffer_bpp;
} __attribute__((packed));

// ... остальная часть кода ...

vbe_init_from_multiboot(
    fb_tag->framebuffer_width,
    fb_tag->framebuffer_height,
    fb_tag->framebuffer_bpp,
    fb_tag->framebuffer_pitch,
    (void*)(uintptr_t)fb_tag->framebuffer_addr
);
EOF

# Пересобираем проект
make clean && make all && make iso && make run
