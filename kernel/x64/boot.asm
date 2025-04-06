bits 32
section .multiboot_header
align 8

header_start:
    dd 0xe85250d6              
    dd 0                       
    dd header_end - header_start 
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; Контрольная сумма

    ; Тег фреймбуфера (тип 5)
    dw 5               
    dw 0                
    dd 32                
    dd 0                
    dd 0                  
    dd 1366               
    dd 768               
    dd 5464               
    db 32                
    db 1                  
    dw 0                
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

    push ebx      
    push eax       
    call kernel_main
    add esp, 8    

    cli
hang:
    hlt
    jmp hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
