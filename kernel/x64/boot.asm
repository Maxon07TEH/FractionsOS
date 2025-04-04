bits 32
section .multiboot_header
align 8  ; Выравнивание всего заголовка

header_start:
    dd 0xe85250d6                ; Magic number (Multiboot2)
    dd 0                         ; Архитектура (i386)
    dd header_end - header_start ; Длина заголовка
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; Контрольная сумма

    ; Тег фреймбуфера (тип 5)
    ; Структура тега (общий размер – 32 байта):
    ;   uint32_t type;
    ;   uint32_t size;
    ;   uint64_t framebuffer_addr;
    ;   uint32_t framebuffer_width;
    ;   uint32_t framebuffer_height;
    ;   uint32_t framebuffer_pitch;
    ;   uint8_t  framebuffer_bpp;
    ;   uint8_t  framebuffer_type;  (1 = RGB)
    ;   uint16_t reserved;
    dw 5                   ; type = 5
    dw 0                   ; Флаги/выравнивание
    dd 32                  ; Размер тега в байтах
    dd 0                   ; framebuffer_addr (низкая часть)
    dd 0                   ; framebuffer_addr (верхняя часть)
    dd 1366                ; framebuffer_width
    dd 768                 ; framebuffer_height
    dd 5464                ; framebuffer_pitch = 1366 * 4
    db 32                  ; framebuffer_bpp (32 бита)
    db 1                   ; framebuffer_type (1 = RGB)
    dw 0                   ; reserved
    align 8

    ; Конечный тег
    dw 0
    dw 0
    dd 8
header_end:

section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top

    push ebx       ; Указатель на структуру Multiboot2
    push eax       ; Magic number
    call kernel_main
    add esp, 8     ; Очистка стека

    cli
hang:
    hlt
    jmp hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
