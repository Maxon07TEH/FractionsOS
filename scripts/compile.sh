#!/bin/bash

ARCH=${1#-}
OBJ_DIR="build/obj"
BIN_DIR="build/bin"

mkdir -p kernel/$ARCH/$OBJ_DIR $OBJ_DIR $BIN_DIR

nasm -f elf32 -g -F dwarf -o kernel/$ARCH/$OBJ_DIR/boot.o kernel/$ARCH/boot.asm || exit 1
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -I./drivers -O2 -Wall -Wextra -c kernel/$ARCH/main.c -o kernel/$ARCH/$OBJ_DIR/main.o || exit 1
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -c drivers/$ARCH/Video/vga/vga.c -o $OBJ_DIR/vga.o || exit 1
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -c drivers/$ARCH/Video/vbe/vbe.c -o $OBJ_DIR/vbe.o || exit 1
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -c src/modules/BinFont.c -o $OBJ_DIR/BinFont.o || exit 1

# Линковка с добавлением стандартной библиотеки
ld -m elf_i386 -T linker.ld -o $BIN_DIR/FractionsOS.$ARCH.bin \
    kernel/$ARCH/$OBJ_DIR/boot.o \
    kernel/$ARCH/$OBJ_DIR/main.o \
    $OBJ_DIR/vga.o \
    $OBJ_DIR/BinFont.o \
    $OBJ_DIR/vbe.o || exit 1
