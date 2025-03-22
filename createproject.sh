#!/bin/bash

# Создаем недостающие директории
mkdir -p drivers scripts boot/grub efi/boot

# Драйвер VGA (drivers/vga.h)
cat > drivers/vga.h << 'EOF'
#ifndef VGA_H
#define VGA_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

enum vga_color {
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GREY = 7,
    COLOR_DARK_GREY = 8,
    COLOR_LIGHT_BLUE = 9,
    COLOR_LIGHT_GREEN = 10,
    COLOR_LIGHT_CYAN = 11,
    COLOR_LIGHT_RED = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_YELLOW = 14,
    COLOR_WHITE = 15,
};

void vga_initialize(void);
void vga_write_string(const char* data);
void vga_set_color(uint8_t fg, uint8_t bg);

#endif
EOF

# Драйвер VGA (drivers/vga.c)
cat > drivers/vga.c << 'EOF'
#include "vga.h"
#include <stdint.h>

static uint16_t* vga_buffer = (uint16_t*)0xB8000;
static size_t vga_row = 0;
static size_t vga_column = 0;
static uint8_t vga_color = 0;

void vga_initialize(void) {
    vga_set_color(COLOR_LIGHT_GREY, COLOR_BLACK);
    for(size_t y = 0; y < VGA_HEIGHT; y++) {
        for(size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = (vga_color << 8) | ' ';
        }
    }
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_color = fg | (bg << 4);
}

void vga_write_string(const char* data) {
    while(*data != '\0') {
        if(vga_column >= VGA_WIDTH) {
            vga_column = 0;
            vga_row++;
        }
        
        if(vga_row >= VGA_HEIGHT) {
            vga_row = 0;
        }

        if(*data == '\n') {
            vga_column = 0;
            vga_row++;
            data++;
            continue;
        }

        const size_t index = vga_row * VGA_WIDTH + vga_column;
        vga_buffer[index] = (vga_color << 8) | *data;

        vga_column++;
        data++;
    }
}
EOF

# Конфигурация GRUB (boot/grub/grub.cfg)
cat > boot/grub/grub.cfg << 'EOF'
set timeout=15
set default=0

menuentry "FractionsOS" {
    multiboot2 /boot/FractionsOS.x64.bin
    boot
}
EOF

# Скрипт компиляции (scripts/compile.sh)
cat > scripts/compile.sh << 'EOF'
#!/bin/bash

ARCH=$1
nasm -f elf64 -o kernel/x64/boot.o kernel/x64/boot.asm
gcc -ffreestanding -m64 -c kernel/x64/main.c -o kernel/x64/main.o
gcc -ffreestanding -m64 -c drivers/vga.c -o drivers/vga.o
ld -T linker.ld -nostdlib -o FractionsOS.x64.bin kernel/x64/boot.o kernel/x64/main.o drivers/vga.o
EOF

# Скрипт создания ISO (scripts/make-iso.sh)
cat > scripts/make-iso.sh << 'EOF'
#!/bin/bash

mkdir -p isodir/boot/grub
cp FractionsOS.x64.bin isodir/boot/
cp boot/grub/grub.cfg isodir/boot/grub/
grub-mkrescue -o FractionsOS.iso isodir
EOF

# Скрипт запуска (scripts/run.sh)
cat > scripts/run.sh << 'EOF'
#!/bin/bash

ARCH=$1
qemu-system-x86_64 -cdrom FractionsOS.iso -m 512M
EOF

# Скрипт очистки (scripts/clean.sh)
cat > scripts/clean.sh << 'EOF'
#!/bin/bash

rm -rf isodir
rm -f *.bin *.iso
rm -f kernel/x64/*.o drivers/*.o
EOF

# Делаем скрипты исполняемыми
chmod +x scripts/*.sh

# Создаем базовую файловую систему FAT64 (пример)
dd if=/dev/zero of=disk.img bs=1M count=64
mkfs.fat -F 64 disk.img
mkdir -p mnt
sudo mount disk.img mnt
sudo mkdir -p mnt/EFI/BOOT
sudo umount mnt
rmdir mnt

echo "Структура проекта создана!"
