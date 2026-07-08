[org 0x7c00]    ; diz ao NASM o endereco onde este codigo vai ser guardado
                ; BIOS carrega sempre o bootloader neste endereco

jmp inicio_codigo       ; salta para o inicio codigo
nop                     ; byte 2 exigido pelo config FAT32
times 90-($-$$) db 0    ; preenches os bytes 3-89 com zeros (reservados para campos FAT32)

inicio_codigo:
    xor ax, ax      ; configurar segmentos para 0
    mov ds, ax      ; DS = 0 (necessario para enderecos funcionarem com org 0x7c00)
    mov es, ax      ; ES = 0 (necessario para INT 13h carregar para ES:BX)

    ; Ler campos do boot Sect
    mov ax, [0x7c00 + 14]                           ; le reservedSects (offset 14, 2 bytes) sectores antes de FAT
    mov [reservedSects], ax 

    mov al, [0x7c00 + 16]                           ; le numFAT (pffset 16, 1 byte) numero de copias FAT
    mov [numFAT], al

    mov eax, [0x7c00 + 36]                          ; le sectsPerFAT (offset 36, 4 bytes) tamanho de cada FAT
    mov [sectsPerFAT], eax

    mov al, [0x7c00 + 13]                           ; le sectsPerCluster (offset 13, 1 byte) sectores por cluster
    mov [sectsPerCluster], al

    mov eax, [0x7c00 + 44]                          ; le rootCluster (offset 44, 4 bytes) - cluster do directorio raiz
    mov [rootCluster], eax

    ; Calcular onde comeca FAT e a area de dados
    movzx eax, word [reservedSects]                 ; fatStart = reservedSects
    mov [fatStart], eax

    movzx ebx, byte [numFAT]                        ; ebx = numFat
    mov eax, [sectsPerFAT]                          ; eax = sectsPerFAT
    mul ebx                                         ; eax = numFAT * sectorsPerFAT
    movzx ebx, word [reservedSects]                 ; ebx = reservedSectors
    add eax, ebx                                    ; dataStart = reservedSectors + numFAT * sectsPerFAT
    mov [dataStart], eax                            ;



load_kernel:
    ; calcular o sector do diretorio raiz (cluster 2)
    mov eax, [rootCluster]                          ; eax = 2
    sub eax, 2                                      ; eax = 0
    movzx ecx, byte [sectsPerCluster]               ; le 1 byte da variavel sectsPerCluster, copia-o para ecx, e preenche os 3 bytes superiores de ecx com zeros
    mul ecx                                         ; eax = 0 * sectoresPerCluster = 0
    add eax, [dataStart]                            ; eax = dataStart + 0 = dataStart

    ; ler 1 sector do diretorio raiz para 0x7E00
    mov bx, 0x7E00              ; endereco de destino longo apos o bootloader
    call lerSectores

procurar_kernel:
    mov si, 0x7E00              ; SI aponta para o inicio do diretorio raiz de memoria
    mov cx, 16                  ; 16 entradas por sector (512 bytes / 32 bytes por entrada)

verificar_entrada:
    mov di, si                  ; DI aponta para a entrada atual do diretorio
    mov bx, nome_kernel         ; BX aponta para o nome que se quer encontrar
    mov dx, 11                  ; 11 bytes a comparar (8 nomes + 3 extensao)

comparar:
    mov al, [di]                ; le o byte de entrada do directorio
    mov ah, [bx]                ; le o byte do nome que procuramos
    cmp al, ah                  ; compara os dois bytes
    jne proxima_entrada         ; se forem diferentes, salta para a funcao proxima entrada
    inc di                      ; avanca na entrada do diretorio
    inc bx                      ; avanca no nome que procuramos
    dec dx                      ; decrementa o contador de bytes
    jnz comparar                ; repete ate comparar os 11 bytes todos

    jmp encontrou_kernel        ; todos os 11 bytes iguais - encontrou-se o ficheiro

proxima_entrada:
    add si, 32                  ; avanca 32 bytes para a proxima entrada do diretorio
    loop verificar_entrada      ; repete as 16 entradas do sector
    jmp disk_error              ; nao encontrou KERNEL.BIN no root logo salta para a funcao disk_error

encontrou_kernel:
    mov ax, [si + 20]                   ; Cluster Alto
    shl eax, 16                         ; desloca 16 bits para cima
    mov ax, [si + 26]                   ; cluster baixo
    mov [clusterAtual], eax             ; guarda o cluster atual (so corre aqui, 1a vez, a partir da entrada de diretorio em [si])

carregar_cluster:                       ; CORRIGIDO: novo label - alvo do loop em vez de "encontrou_kernel"
                                         ; assim o loop deixa de voltar a ler [si] (que so tem o 1o cluster) e usa o cluster ja actualizado pela FAT
    mov eax, [clusterAtual]             ; carrega o cluster atual em EAX (1a vez: veio de [si]; seguintes: veio da FAT)
    sub eax, 2                          ; subtrai 2 pois os clusters 0 e 1 sao reservados
    movzx ecx, byte [sectsPerCluster]   ; numero de sectores por cluster com zero extension
    mul ecx                             ; (cluster - 2) * sectorsPerCluster = offset desde o inicio da area de dados
    add eax, [dataStart]                ; soma o inicio da area de dados para obter o sector absoluto do disco

    mov bx, 0x8000                      ; endereco de destino na RAM onde o kernel vai ser carregado
    movzx ecx, byte [sectsPerCluster]   ; numero de sectores a ler (um cluster inteiro)

ler_cluster:
    push eax            ; guarda o sector atual
    push ecx            ; guarda o contador
    call lerSectores    ; le 1 sector do disco para o endereco em BX
    pop ecx             ; restaura o contador
    pop eax             ; restaura o sector atual
    add bx, 512         ; avanca o destino 512 bytes
    inc eax             ; avanca para o proximo sector
    loop ler_cluster    ; repete ECX vezes

    ; ler o proximo cluster de FAT
    mov eax, [clusterAtual]     ; cluster atual
    shl eax, 2                  ; shl - Shift Left isto desloca os bits para a esquerda. Cada deslocamento para a esquerda de 1 bit é equivalente a multiplicar por 2, logo se for 2 vamos multiplicar por 4
    mov ecx, 512                
    xor edx, edx
    div ecx                     ; eax = sector dentro de FAT, edx = offset dentro do sector 

    add eax, [fatStart]         ; sector absoluto de FAT
    push edx                    ; guarda o offset
    mov bx, 0x6000              ; le o sector da FAT para 0x6000 (temporario)
    call lerSectores
    pop edx                     ; restaura o offset

    mov eax, [0x6000 + edx]     ; le o proximo cluster
    and eax, 0x0FFFFFFF         ; mascara os 4 bits superiores (ignorados em FAT32)

    cmp eax, 0x0FFFFFF8         ; fim do ficheiro ?
    jae saltar_kernel           ; se sim salta para 0x8000

    mov [clusterAtual], eax     ; se nao atualiza o cluster atual
    jmp carregar_cluster        ; CORRIGIDO: era "jmp encontrou_kernel", que relia [si] e destruia o cluster novo -
                                 ; agora repete o loop sem tocar em [si], preservando o cluster seguinte da cadeia

saltar_kernel:
    jmp 0x0000:0x8000           ; salta para a kernel

disk_error:
    mov al, 'E'     ; Vai guardar E em AL
    mov ah, 0x0e    ; Vai usar usar a funcao para imprimir 
    int 0x10        ; vai executar
    jmp $           ; vai fazer o loop

dap:
    db 0x10     ; tamanho da DAP (16 bytes)
    db 0        ; reservado
    dw 1        ; numero sectores a ler
    dw 0        ; offset do destino (preenchido atens de ser chamado)
    dw 0        ; segmento do destino (sempre 0)
    dd 0        ; LBA baixo (32 bits)
    dd 0        ; LBA alto (32 bits, sempre 0)

; Le o 1o sector do disco
; entrada: eax = LBA, bx = endereco do destino
lerSectores:
    push si             ; guarda o registo de SI na stack
    mov [dap + 4], bx   ; destino do offset e preenche-o
    mov [dap + 8], eax  ; LBA baixo e preenche-o
    mov si, dap         ; carrega o endereco do DAP em SI
    mov ah, 0x42        ; Extended Read Sectors
    mov dl, 0x80        ; Especifica o disco, neste caso o disco rigido
    int 0x13            ; Executa um pedido a BIOS
    jc disk_error       ; Se houver um erro, salta para a funcao disk_error
    pop si              ; Restaura o valor de SI
    ret                 ; Volta para onde a funcao foi chamada

jmp $           ; prende a CPU num loop infinito
                ; este faz-lo pois nao ha nada abaixo e sem ele podem se corromper ficheiros
                ; tipo um while(1) em C

reservedSects dw 0
numFAT db 0
sectsPerFAT dd 0
sectsPerCluster db 0
rootCluster dd 0
fatStart dd 0
dataStart dd 0
nome_kernel db "KERNEL  BIN"
clusterAtual dd 0

times 510-($-$$) db 0   ; enche o espaco vazio no sector cheiso de 0 até ao byte 510
                        ;  $ = posicao atual : $$ = inicio de uma seccao : basicamente significa quantos bytes ja escrevi
                        ; ate ao 510 pois o 510 e 511 fazem parte do magic number, um endereco onde a BIOS vai automaticamente para verificar se o programa e bootable
                        
dw 0xaa55       ;escreve esse numero magico
                ;BIOS vai verificar se e bootable

; 0x7c00 = Endereco do bootloader
; 0x0e = Modo de imprimir 
; 0x10 = Funcao de video
