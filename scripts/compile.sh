#!/bin/bash
ARCH=${1#-}
OBJ="build/obj"
BIN="build/bin"
mkdir -p kernel/$ARCH/$OBJ $OBJ $BIN

nasm -f elf32 -g -F dwarf -o kernel/$ARCH/$OBJ/boot.o kernel/$ARCH/boot.asm
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -I./drivers -O2 -Wall -c kernel/$ARCH/main.c -o kernel/$ARCH/$OBJ/main.o
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -c drivers/x64/Video/vga/vga.c        -o $OBJ/vga.o
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -c drivers/x64/Interrupt/io.c        -o $OBJ/io.o
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -c drivers/x64/Interrupt/pic.c       -o $OBJ/pic.o
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -c drivers/x64/Interrupt/interrupts.c -o $OBJ/interrupts.o
gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -c drivers/x64/Interrupt/scheduler.c  -o $OBJ/scheduler.o
nasm -f elf32 drivers/x64/Interrupt/isr_stubs.S -o $OBJ/isr_stubs.o

ld -m elf_i386 -T linker.ld -o $BIN/FractionsOS.$ARCH.bin \
    kernel/$ARCH/$OBJ/boot.o \
    kernel/$ARCH/$OBJ/main.o \
    $OBJ/vga.o \
    $OBJ/io.o \
    $OBJ/pic.o \
    $OBJ/interrupts.o \
    $OBJ/scheduler.o \
    $OBJ/isr_stubs.o
