[org 0x8000]

mov ah, 0x00        ; Ativa o modo gráfico
mov al, 0x13        ; Vai usar o modo 0x13 que tem 256 cores em um 320x200
int 0x10            ; BIOS executa - o ecra muda para o mod grafico

mov ax, 0xA000              ; O framebuffer do VGA inicia em 0xA000
mov es, ax                  ; parte de 0xA000 que esta a apontar para o inicio do framebuffer

; retangulo vermelho em (x=100, y=50), largura=80, altura=40
mov di, (50 * 320) + 100
mov bx, 40                  ; 40 linhas

linha_loop:
    push cx              ; guarada cx antes do loop interno
    mov cx, 80           ; 80 pixels por linha
    mov al, 4            ; faz com que al seja vermelho

    pixel_loop:
        mov [es:di], al ; desde o inicio do framebuffer ate di faz com que os pixeis sejam vermelhos
        inc di          ; increment di
        loop pixel_loop ; faz com que pixel_loop fique num ciclo 

    pop cx              ; restaura cx
    add di, 320 - 80    ; salta para o inicio da proxima linha
    dec bx              ; decrementa bx
    jnz linha_loop      ; se bx != 0 -> volta ao inicio do loop

jmp $