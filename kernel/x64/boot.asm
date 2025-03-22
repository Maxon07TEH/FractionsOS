bits 32
section .multiboot_header
align 8
header_start:
    dd 0xe85250d6                ; Magic number (Multiboot 2)
    dd 0                         ; Architecture (i386)
    dd header_end - header_start ; Header length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; Checksum

    ; End tag
    dw 0    ; Type
    dw 0    ; Flags
    dd 8    ; Size
header_end:

section .text
global _start
_start:
    mov esp, stack_top
    extern kernel_main

    ; Проверка Multiboot
    call check_multiboot
    call kernel_main

    cli
global hang  ; Делаем метку видимой
hang:
    hlt
    jmp hang

check_multiboot:
    cmp eax, 0x36d76289
    jne .no_multiboot
    ret
.no_multiboot:
    mov al, '0'
    jmp hang  ; Используем глобальную метку

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
