bits 32
section .multiboot_header
align 8  ; Выравнивание всего заголовка

header_start:
    dd 0xe85250d6                ; Magic
    dd 0                         ; Архитектура
    dd header_end - header_start ; Длина
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; Контрольная сумма

    ; Тег фреймбуфера (тип 5)
    dw 5
    dw 0 ; Убираем фиксированные настройки
    dd 20
    dd 0 ; Ширина
    dd 0 ; Высота
    dd 0 ; Глубина цвета
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
