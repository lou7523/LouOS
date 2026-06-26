section .text.boot
global inicio
extern kernel_main

[bits 16]
inicio:
    cli

    xor ax, ax
    mov ds, ax
    mov es, ax

    o32 lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp dword 0x08:inicio_32bit

[bits 32]
inicio_32bit:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    lidt [idt_descriptor]

    call kernel_main
    jmp $

; GDT - Global Descriptor Table
gdt_inicio:
    
    dq 0 ; entrada nula - obrigatoria

    ; entrada 1 - segmento de codigo 32 bits
    dw 0xFFFF       ; limites (bits 0-15)
    dw 0x0000       ; base (bits 0-15)
    db 0x00         ; base (bits 16-23)
    db 10011010b    ; acesso: presente, codigo 
    db 11001111b    ; flags: 32 bits, limite bits 16-19
    db 0x00         ; base (24-31)

    ; entrada 2 - segmento de dados 32 bits
    dw 0xFFFF       ; limites (bits 0-15)
    dw 0x0000       ; base (bits 0-15)
    db 0x00         ; base (bits 16-23)
    db 10010010b    ; acesso: presente, codigo
    db 11001111b    ; flags: 32 bits, limite bits 16-19
    db 0x00         ; base (24-31)

gdt_fim:

gdt_descriptor:
    dw gdt_fim - gdt_inicio - 1     ; tamanho do GDT
    dd gdt_inicio                   ; Endereco do GDT

idt_descriptor:
    dw 0
    dd 0

; --- Dados ---
    texto db 'LouOS', 0
    fonte_off dw 0
    fonte_seg dw 0
    cursor_x dw 0
    cursor_y dw 0
