; drivers/x64/Interrupt/isr_stubs.S
BITS 32

; Таблица векторов: 256 записей, указывающих на общий обработчик
global isr_stub_table
isr_stub_table:
    times 256 dd isr_common_stub

; Общий обработчик прерывания
global isr_common_stub
isr_common_stub:
    pusha                   ; сохранить все регистры
    push ds
    push es
    mov ax, 0x10
    mov ds, ax              ; DS = селектор сегмента данных
    mov es, ax              ; ES = селектор сегмента данных

    ; --- здесь можно вызвать C‑функцию, если нужно ---
    ; call isr_handler_dispatch

    ; Отправляем EOI мастер- и слейв‑PIC
    mov al, 0x20            ; код EOI
    out 0x20, al            ; мастер PIC
    out 0xA0, al            ; слейв PIC

    pop es
    pop ds
    popa                    ; восстановить регистры
    add esp, 4              ; пропустить код ошибки (или выровнять стек)
    iret                    ; возврат из прерывания
