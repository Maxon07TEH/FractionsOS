#!/bin/bash

ARCH=${1#-}
OBJ_DIR="obj"

# Создаем директории для объектных файлов
mkdir -p kernel/x64/$OBJ_DIR drivers/$OBJ_DIR

# Компиляция ASM с явным указанием формата
nasm -f elf64 -o kernel/x64/$OBJ_DIR/boot.o kernel/x64/boot.asm || exit 1
gcc -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
    -I./drivers -c kernel/x64/main.c -o kernel/x64/$OBJ_DIR/main.o
gcc -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
    -c drivers/vga.c -o drivers/$OBJ_DIR/vga.o

ld -n -T linker.ld -o FractionsOS.x64.bin \
    kernel/x64/$OBJ_DIR/boot.o \
    kernel/x64/$OBJ_DIR/main.o \
    drivers/$OBJ_DIR/vga.o
