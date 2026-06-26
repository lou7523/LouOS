[org 0x8000]

jmp inicio

; --- Dados ---
    texto db 'LouOS', 0
    fonte_off dw 0
    fonte_seg dw 0
    cursor_x dw 0
    cursor_y dw 0

inicio:
    ; Ativa modo grafico 320x200, 256 cores
    mov ah, 0x00
    mov al, 0x13
    int 0x10

    ; Aponta ES para o framebuffer VGA
    mov ax, 0xA000
    mov es, ax

    ; Preenche o ecra todo de azul
    mov di, 0
    mov cx, 320 * 200
    mov al, 1

fundo_loop:
    mov [es:di], al
    inc di
    loop fundo_loop

    ; Barra de tarefas cinzenta no fundo (y=180, altura=20)
    mov di, (180 * 320)
    mov bx, 20

barra_loop:
    mov cx, 320
    mov al, 7

    barra_pixel_loop:
        mov [es:di], al
        inc di
        loop barra_pixel_loop

    dec bx
    jnz barra_loop

    ; Obtem o endereco da fonte 8x8 da BIOS (bh=0x03 = fonte 8x8, chars 0-127)
    mov ax, 0x1130
    mov bh, 0x03
    int 0x10

    ; Guarda segmento e offset da fonte
    mov [fonte_seg], es
    mov [fonte_off], bp

    ; Restaura ES para o framebuffer
    mov ax, 0xA000
    mov es, ax

    ; Prepara para desenhar "LouOS" na posicao (10, 5)
    mov si, texto
    mov word [cursor_x], 10
    mov word [cursor_y], 5

desenhar_texto:
    mov al, [si]
    cmp al, 0
    je fim_texto

    call desenhar_char
    add word [cursor_x], 8      ; avanca 8 pixeis para a direita
    inc si
    jmp desenhar_texto

fim_texto:
    jmp $                       ; loop infinito - kernel fica aqui

; --- Desenha um caracter 8x8 na posicao (cursor_x, cursor_y) ---
; Entrada: AL = codigo ASCII do caracter
desenhar_char:
    push ax
    push bx
    push cx
    push dx
    push si
    push di

    ; Calcula offset na tabela de fonte (char * 8)
    mov ah, 0
    mov cx, 8
    mul cx

    ; Aponta SI para os bytes do caracter na fonte
    mov si, [fonte_off]
    add si, ax

    ; Carrega o segmento da fonte em FS
    push word [fonte_seg]
    pop fs

    ; Restaura ES para o framebuffer
    mov ax, 0xA000
    mov es, ax

    ; Calcula posicao no framebuffer: y * 320 + x
    mov ax, [cursor_y]
    mov cx, 320
    mul cx
    add ax, [cursor_x]
    mov di, ax

    mov cx, 8                   ; 8 linhas por caracter

linhas_loop:
    push cx
    mov bl, [fs:si]             ; le uma linha da fonte (8 bits = 8 pixeis)
    inc si
    mov cx, 8                   ; 8 pixeis por linha

bits_loop:
    test bl, 0x80               ; testa o bit mais significativo
    jz pixel_vazio
    mov byte [es:di], 15        ; branco se o bit esta ativo
    jmp proximo_bit

pixel_vazio:
    mov byte [es:di], 1         ; azul (fundo) se o bit esta inativo

proximo_bit:
    shl bl, 1                   ; proximo bit
    inc di
    loop bits_loop

    add di, 320 - 8             ; salta para a proxima linha no ecra
    pop cx
    loop linhas_loop

    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret
