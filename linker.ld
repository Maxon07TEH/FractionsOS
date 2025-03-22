ENTRY(_start)

SECTIONS {
    . = 1M;
    .text : ALIGN(4K) {
        *(.multiboot_header)
        *(.text)
    }
    .rodata : ALIGN(4K) { *(.rodata) }
    .data : ALIGN(4K) { *(.data) }
    .bss : ALIGN(4K) {
        *(COMMON)
        *(.bss)
        . = ALIGN(16);
        stack_bottom = .;
        . += 16K;
        stack_top = .;
    }
}
