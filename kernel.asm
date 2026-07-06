section .text.boot              ;diz ao linker para colocar o .text.boot ficar no inicio do binario final
global inicio                   ;tornam estas duas funcoes visiveis fora deste ficheiro
global keyboard_handler
global tss_descriptor
global saltarRing3 
global syscallHandler    
extern keyboard_handler_c       ;Diz ao assembler que estas funcoes estao noutro ficheiro
extern kernel_main
extern syscallHandlerC

[bits 16]                   ;inicio do modo 16bits
;inicio da funcao inicio
inicio:
    cli                     ;Clear Interrupt Flag, desliga as interrupcoes para nao ser interrompido por nenhum hardware

    xor ax, ax              ;poe AX a 0
    mov ds, ax              ;copiar o DataSegment (ds) 
    mov es, ax              ;copiar o ExtraSegment (es)
    ;temos de fazer isto pois o bootloader assume enderecos a partir de 0x7c00 com segmentos a 0, sem isto
    ;os enderecos calculados mais a frente iriam ser errados

    mov di, 0x2000          ;di vai apontar para o mapa de memoria da BIOS
    xor ebx, ebx            ;ebx vai ser o marcado de posicao que a BIOS vai usar para saber onde ficou entre uma chamada e a seguinte

ler_mapa:
;inicio da funcao

    mov eax, 0xE820         ;Diz a BIOS que funcao int 0x15 quero
    mov ecx, 20             ;Intrucao que diz a BIOS que cada bloco tem cerca de 20 bytes
    mov edx, 0x534D4150     ;A palavra passe fixa SMAP em ASCII, esta e exigida
    int 0x15                ;Executa o pedido a BIOS

    jc fim_mapa         ;Jump If Carry, verifica se a chamada falhou e salta para fora do ciclo caso aconteca algum erro

    add di, 20          ;Avanca-se 20 bytes 
    
    cmp ebx, 0          ;se ebx for 0, para verificar se ainda a blocos
    je fim_mapa         ;Jump if Equal caso sim

    jmp ler_mapa        ;volta-se ao inicio da funcao

fim_mapa:
;Funcao fim mapa
    xor eax, eax
    mov [di],       eax
    mov [di+4],     eax
    mov [di+8],     eax
    mov [di+12],    eax
    mov [di+16],    eax

    o32 lgdt [gdt_descriptor]       ; Carrega o GDT na CPU
    mov eax, cr0                    ; cr0 e um registo para o controlo da CPU, o bit 0 = PE (Protection Enable), e ativa-lo e o que liga o modo 32 bits
    or eax, 1                       ; vai ler esse bit
    mov cr0, eax                    ; vai liga-lo sem mexer nos outros bits
    jmp dword 0x08:inicio_32bit     ; Um salto far jump não só muda o EIP (próxima instrução), como também recarrega CS (Code Segment) para 0x08, 
                                    ; que é o selector do segmento de código 32-bit na GDT. Isto é o que finaliza a transição,
                                    ; o CPU só "acredita" mesmo que está em modo protegido depois deste salto.

[bits 32]                           ; A partir daqui o NASM escreve as instrucoes em 32 bits
inicio_32bit:
    mov ax, 0x10                    ; 0x10 e o seletor do segmento de dados na minha GDT(segunda entrada)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000                ; Define o topo do stack onde push/pop/call/ret vao ser guardados -> 0x90000

    call kernel_main            ; Chama a funcao em C
    jmp $                       ; ajuda a CPU a ficar num loop infinito

keyboard_handler:
    pusha                       ; salva os registos todos
    call keyboard_handler_c     ; chama a funcao keyboard handler c
    popa                        ; restaura todos os registos
    mov al, 0x20                ; Avisa o PIC que terminamos a interrupcao
    out 0x20, al                ; permite-nos avancar para a prox
    iret                        ; restaura o EIP, CS e EFLAGS

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

    ; entrada 3 - cdigo do Ring 3
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 11111010b    ; acesso: presente, Ring 3, dados
    db 11001111b    ; flags: 32 bits, limite bits 16-19
    db 0x00

    ; entrada 4 - dados do Ring 3
    dw 0xFFFF   
    dw 0x0000
    db 0x00
    db 11110010b    ; acessp: presente, Ring 3, dados
    db 11001111b    ; flags: 32 bits, limite bits 16-19
    db 0x00

tss_descriptor:
    dw 0x0067          ; tamanho da TSS: 104 - 1 = 103 = 0x67
    dw 0x0000           ; base bits 0-15 (preenchido em runtime)
    db 0x00             ; base bits 16-23 (preenchido em runtime)
    db 10001001b        ; tipo: TSS disponivel (0x89)
    db 000000000        ; flags
    db 0x00             ; base bits 24-31 (preenchido em runtime)

gdt_fim:

gdt_descriptor:
    dw gdt_fim - gdt_inicio - 1     ; tamanho do GDT
    dd gdt_inicio                   ; Endereco do GDT

saltarRing3:
    mov eax, [esp + 4]
    mov bx, 0x23            ; seleciona o Ring 3
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    push 0x23               ; ss - stack segment Ring 3
    push 0x7FF000           ; esp - stack do programa em Ring 3
    pushf                   ; eflgas - flags atuais
    push 0x1B               ; cs - seletor codigo Ring 3
    push eax                ; eip - endereco passado como argumento
    iret                    ; restaura estes valores

syscallHandler:
    pusha
    call syscallHandlerC
    popa
    iret