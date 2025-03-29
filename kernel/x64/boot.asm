bits 32
section .multiboot_header
align 8  ; Выравнивание всего заголовка

header_start:
    dd 0xe85250d6                ; Magic
    dd 0                         ; Архитектура
    dd header_end - header_start ; Длина
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; Контрольная сумма

    ; Тег фреймбуфера (тип 5)
    ; Структура тега (общий размер – 32 байта):
    ;   uint32_t type;                ; 4 байта (у нас: dw 5 + dw 0)
    ;   uint32_t size;                ; 4 байта
    ;   uint64_t framebuffer_addr;    ; 8 байт
    ;   uint32_t framebuffer_width;   ; 4 байта
    ;   uint32_t framebuffer_height;  ; 4 байта
    ;   uint32_t framebuffer_pitch;   ; 4 байта
    ;   uint8_t  framebuffer_bpp;     ; 1 байт
    ;   uint8_t  framebuffer_type;    ; 1 байт (1 = RGB)
    ;   uint16_t reserved;            ; 2 байта
    dw 5                   ; type = 5 (нижние 16 бит)
    dw 0                   ; type = 0 (верхние 16 бит) или флаги – для выравнивания
    dd 32                  ; размер тега в байтах
    dd 0                   ; framebuffer_addr (низкая часть; загрузчик заполнит или оставит 0, если использовать встроенные настройки)
    dd 0                   ; framebuffer_addr (верхняя часть)
    dd 1366                ; framebuffer_width  (например, 1024)
    dd 768                 ; framebuffer_height (например, 768)
    dd 4                   ; framebuffer_pitch  (например, 1024 пикселей * 4 байта)
    db 32                  ; framebuffer_bpp      (32 бита)
    db 1                   ; framebuffer_type   (1 = RGB)
    dw 0                   ; reserved (для выравнивания)
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

    ; Передача параметров Multiboot2 в kernel_main
    push ebx       ; Указатель на структуру Multiboot2
    push eax       ; Magic number (0x36d76289)
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
