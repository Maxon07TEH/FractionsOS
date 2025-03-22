#!/bin/bash

ARCH=${1#-}
OBJ_DIR="obj"

mkdir -p kernel/x64/$OBJ_DIR drivers/$OBJ_DIR

nasm -f elf32 -g -F dwarf -o kernel/x64/$OBJ_DIR/boot.o kernel/x64/boot.asm || exit 1
gcc -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -I./drivers -c kernel/x64/main.c -o kernel/x64/$OBJ_DIR/main.o || exit 1
gcc -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -c drivers/vga.c -o drivers/$OBJ_DIR/vga.o || exit 1

ld -m elf_i386 -T linker.ld -nostdlib -z max-page-size=0x1000 -o FractionsOS.x64.bin \
    kernel/x64/$OBJ_DIR/boot.o \
    kernel/x64/$OBJ_DIR/main.o \
    drivers/$OBJ_DIR/vga.o || exit 1
