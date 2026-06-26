ENTRY(inicio)

SECTIONS {
    . = 0x8000;

    .text : {
        *(.text.boot)
        *(.text)
        *(.rodata*)
        *(.data)
        *(.bss)
    }

    /DISCARD/ : {
        *(.eh_frame)
        *(.comment)
        *(.note*)
    }
}