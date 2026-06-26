ENTRY(kernel_main)

SECTIONS {
    . = 0x8000;

    .text : {
        kernel.o(.text)
        *(.text)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }
}