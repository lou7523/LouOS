[org 0x7c00]    ; diz ao NASM o endereco onde este codigo vai ser guardado
                ; BIOS carrega sempre o bootloader neste endereco

mov ah, 0x0e    ; define AH no 0x0e = modo para imprimir caracteres
                ; AH é a parte 'mais alta' do AX
                ; 0x0e diz a BIOS para interromper o que se passa em 0x10 em avanco

mov al, 'l'     ;mete o char 'L' na parte mais baixa de AX (AL)
                ; AL e o que se le para saber qual char imprimir
int 0x10        ; chama a funcao de videos da BIOS e imprime a o que esta guardado em AL

mov al, 'o'     ; carrega 'o' para AL
int 0x10        ; imprime AL

mov al, 'u'
int 0x10

mov al, 'r'
int 0x10

mov al, 'e'
int 0x10

mov al, 'n'
int 0x10

mov al, 'c'
int 0x10

mov al, 'o'
int 0x10

load_kernel:
    mov ah, 0x02         ; Funcao de ler sectores
    mov al, 5            ; qunatos sectores desejas ler (5 x 512 = 2560)
    mov ch, 0            ; o setor 2 esta no cilindro 0
    mov cl, 2            ;o sector comeca em 1 e nao em 0. o sector 1 e o boot sector por isso a kernel comeca no 2
    mov dh, 0            ; a cabeca tem 2 faces (cima/baix) = cima, normalmente e usada sempre para os primeiros setores
    mov bx, 0x8000       ; endereco destino a memoria RAM. A BIOS vai copiar os seguintes setores lidos para este espaco de memoria
    int 0x13             ; executa tudo o que esta configurado aqui em cima
    jmp 0x0000:0x8000    ; Vai saltar para o endereco onde esta a kernel

int 0x13            ; vai executar
jc disk_error       ; se nao der erro vai -> executar em 0x8000

jmp 0x0000:0x8000   ; executa a kernel em 0x8000

disk_error:
    mov al, 'E'     ; Vai guardar E em AL
    mov ah, 0x0e    ; Vai usar usar a funcao para imprimir 
    int 0x10        ; vai executar
    jmp $           ; vai fazer o loop

jmp $           ; prende a CPU num loop infinito
                ; este faz-lo pois nao ha nada abaixo e sem ele podem se corromper ficheiros
                ; tipo um while(1) em C

times 510-($-$$) db 0   ; enche o espaco vazio no sector cheiso de 0 até ao byte 510
                        ;  $ = posicao atual : $$ = inicio de uma seccao : basicamente significa quantos bytes ja escrevi
                        ; ate ao 510 pois o 510 e 511 fazem parte do magic number, um endereco onde a BIOS vai automaticamente para verificar se o programa e bootable
                        
dw 0xaa55       ;escreve esse numero magico
                ;BIOS vai verificar se e bootable

; 0x7c00 = Endereco do bootloader
; 0x0e = Modo de imprimir 
; 0x10 = Funcao de video
