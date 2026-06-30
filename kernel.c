extern void keyboard_handler();
void pic_init();
void idt_init();
void idt_set(int numero, unsigned int handler);


static inline void outb(unsigned short porto, unsigned char valor) {
    //static inline - diz ao compilador para copiar esta função diretamente para onde e chamada
    __asm__ volatile ("outb %1, %0" : : "Nd"(porto), "a"(valor));
    //asm volatile - permite-me escrever assembly dentro de C
    //outb %1 %0 - instrução em assembly
    //: : Nd (porto) a (valor) - a = registo AL/AX/EAX -> o valor vai para AL
                              //Nd = nº do porto -> porto vai deretamente para a instrucao
    //outb do x86 escreve um byte num Porto de hardware.
    //Naturalmente C não tem esta capacidade, logo pede-se diretamente à CPU
}

void kernel_main() {
    char* video = (char*) 0xB8000;
    int i;
    for (i = 0; i < 80 * 25 * 2; i += 2) {
        video[i] = ' ';
        video[i + 1] = 0x0F;
    }

    char* msg = "LouOS";
    i = 0;
    while (msg[i] != 0) {
        video[i * 2] = msg[i];
        video[i * 2 + 1] = 0x0F;
        i++;
    }

    pic_init();
    idt_init();
    idt_set(0x21, (unsigned int) keyboard_handler);

    __asm__ volatile ("sti"); //liga interrupçoes 
    //sem isto o CPU vai ignorar o PIC, mesmo com o IDT inicializado corretamente

    while(1);
}

void pic_init() {
    //ICW1- Inicia o remapeamento outb - OutputByte
    outb(0x20, 0x11); // 0x20 e o ponto PIC mestre, 0x11 significa que vai dar movas instrucoes, prepara-te
    outb(0xA0, 0x11); // 0x0A e o porto PIC escravo, 0x11 significa que vai dar movas instrucoes, prepara-te
    //ICW2 - Onde comecao os vetores
    outb(0x21, 0x20); // 0x21 e onde ele recebe as instrucoes de 0x20
    outb(0xA1, 0x28); // As IRQs comecao no vetor 0x20 e o escravo comeca no 0x28
    //ICW3 - ligar o mmestre ao escravo
    outb(0x21, 0x04); //Diz ao mestre que tem um escravo ligado na IRQ2
    outb(0xA1, 0x02); //Diz ao escravo que está ligado ao mestre
    //ICW4 - modo 8086 (modo normal de 32bits)
    outb(0x21, 0x01); //Mete o mestre em modo de 32-bits
    outb(0xA1, 0x01); //Mete o escravo em modo de 32-bits
    //Máscaras - bloquear tudo excepto o teclado, este e um byte onde cada bit corresponde a uma IRQ 0 = permitir 1 = bloquear
    outb(0x21, 0xFD); //0xFD em binario e 1111 1101, onde o bit 1 e 0 - IRQ1 é permitida. Tudo o resto e bloqueado
    outb(0xA1, 0xFF); //0xFF bloqueia todas as IRQs do PIC escravo
    
    /*
        PIC - O CPU só tem uma linha de interrupção. 
              Mas tens dezenas de dispositivos que podem precisar da sua atenção ao mesmo tempo,
              teclado, rato, disco, timer, porta série.
              Como é que o CPU sabe quem chamou? E se dois chamarem ao mesmo tempo, 
              qual tem prioridade?
              O CPU sozinho não consegue gerir isso. Por isso existe o PIC.

              PIC é um chip que conecta estas entradas todas a CPU, este tem 8 entradas (IRQ0 - IRQ7)
              mas como estas 8  entradas nao estao chegam para todos os dispositivos, 
              existe um segundo PIC, o escravo, 
              este esta conectado a IRQ2 do 1º PIC, dando mais 8 entradas

        IRQ - Interrupt ReQuest, este e um sinal eletrico entre um dispositivo e o PIC.
              Quando o certo dispositivo quer atençao este puxa esse fio para cima.
              PIC vê o fio ativo e avisa a CPU

      PORTO - Número especifico de cada seccao

        ICW - Internalization Command Word, o PIC nao se configura com um comando, 
              precisa de seguir um protocolo fixo

       8086 - Este foi o primeiro CPU da Intel para PCs, em 78. Este definiu como a CPU
              comunica com o hardware, incluindo o protocolo PIC
              Quando se diz ao PIC que ICW4 = 0x01 estamos a pedir para utilizar o modo 8086
              e nao o modo MCS-80 (mais antigo)
    */
}

struct idt_entrada {
    unsigned short enderecoBaixo;   // bits 0-15
    unsigned short selector;        // sempre 0x08
    unsigned char zero;             // sempre 0
    unsigned char tipo;             // sempre 0x8E
    unsigned short enderecoAlto;    // bits 16-31 do handler
} __attribute__((packed));   //impede a CPU de ler lixo

struct idt_entrada idt[256];

struct idt_descriptor {
    unsigned short tamanho;     //tamanho da tela em bytes - 1
    unsigned int endereco;      //endereco da tabela na RAM
} __attribute__ ((packed));

struct  idt_descriptor idt_desc;

void idt_set(int numero, unsigned int handler) {
    idt[numero].enderecoBaixo = handler & 0xFFFF;           //le os primeiros 16 bits do endereco
    idt[numero].selector = 0x08;
    idt[numero].zero = 0;
    idt[numero].tipo = 0x8E;                                //Este vai dizer a CPU que tipo de entrada é, esta tem sempre o valor fixo de 0x8E;
    idt[numero].enderecoAlto = (handler >> 16) & 0xFFFF;    //desloca o endereco 16 bits para a direita, ficando com os ultimos 16bits
}

void idt_init() {
    idt_desc.tamanho = sizeof(idt) - 1;
    idt_desc.endereco = (unsigned int) idt;


    __asm__ volatile ("lidt %0" : : "m"(idt_desc));
    //lidt - instrucao em assembly que conecta o decriptor da IDT na GPU
}

static inline unsigned char inb(unsigned short porto) {
    unsigned char resultado;
    __asm__ volatile ("inb %1, %0" : "=a"(resultado) : "Nd"(porto));
    return resultado;
}

/*
    O que e um scancode?

    Um scancode e a posicao real e fisica de uma tecla em um teclado, logo
    quando clicamos o teclado nao envia a tecla "a", mas sim o scancode.
*/

//Tabela de conversao scancode -> caracter
    char scancodeParaAscii[] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        '*', 0, ' '
    };

    int cursor_pos = 10;                                        //Ond eo user comeca a escrever
    int shiftPressed = 0;
    void keyboard_handler_c(){                                  //funcao que e chamada em assembly
        unsigned char scancode = inb(0x60);                     //Le o byte 1 do porto 0x60
        char* video = (char*) 0xB8000;                          //Endereco fixo da memoria grafica
        if (scancode == 0xAA || scancode == 0xB6) {             //se SHIFT (esq/dir) for largado 
            shiftPressed = 0;                                   //Shift Pressed = 0
        } else if (scancode < 128) {                            //Para nao digitar uma tecla quando e largada so quando pressionada
            if (scancode == 0x2A || scancode == 0x36) {
                shiftPressed = 1;
            }
            char c = scancodeParaAscii[scancode];               //Traduz o numero scancode para o char correspondente
            if (shiftPressed == 1 && c >= 97 && c <=122) {      //se ShiftPressed = 1, e c>97 e c<122, c for a ate z
                c -= 32;                                        //faz c-32 = as letras serem maiusculas
            } 
            if (c == '\b') {                                    //se c == backspace, vai executar a funcao de apagar
                if (cursor_pos > 0) {   
                    cursor_pos--;
                    video[cursor_pos * 2] = ' ';
                    video[cursor_pos * 2 + 1] = 0x0F;
                }
            } else if (c == '\n'){                                          //e se c == espaco, vai executar a funcao de espaco
                cursor_pos = (cursor_pos / 80 + 1) * 80;                                            
            } else if (c != 0){                                            //senao vai escrever
                video[cursor_pos * 2] = c;                                 //posicao do curso x 2 vai ser igual a c
                video[cursor_pos * 2 + 1] = 0x0F;                          // vai fazer a posicao a frante ter Fpreto e Lbranca
                cursor_pos++;                                              //incrementa a variavel cursor_pos
            }
            if (cursor_pos >= 80 * 25) {                        //se a posicao do cursor_pos >= 80*25 = 4000
                scroll();                                         //vai chamar a funcao scroll 
                cursor_pos = 1920;                                //e vai meter a posicao do cursor a 1920
            }
        }
    }

    void scroll() {
        char* video = (char*) 0xB8000;  //endereco de memoria grafica
        int i;

        for (i = 0; i < 80 * 24 * 2; i++ ) {        //comeca no caracter 0, ate a linha 24, e vai incrementando i para mudar de linha
            video[i] = video[i + 160];              //enquanto isso faz, o byte na posicao i passa a ter o valor que estava na posicao i + 160 (para copiar)
        }

        for (i = 3840; i < 80 * 25 * 2; i += 2) {   //comeca no caracter 3840 (24 linha), ate a linha 25, e vai adicionar 2 bytes ao i
            video[i] = ' ';                         //enquanto i aumenta faz espaco
            video[i + 1] = 0x0F;                    //enquanto i + 1 aumenta e faz o fundo preto e o txt branco
        }
    };

    struct blocoMemoria {
        unsigned long long enderecoBase;
        unsigned long long tamanho;
        unsigned int tipo;
    } __attribute__((packed));

    void lerMapaMemoria() {
        struct blocoMemoria* mapa = (struct blocoMemoria*) 0x9000;

        int i = 0;
        while(mapa[i].tipo != 0) {
            unsigned int tipoAtual = mapa[i].tipo;
            i++;
        }
    };

    void imprimirNumero(int numero, int posicao) {
        char* video = (char*) 0xB8000;
        char buffer[10];
        int i = 0;

        if (numero == 0) {
            video[posicao * 2] = '0';
            video[posicao * 2 + 1] = 0x0F;
            return;
        }

        while (numero > 0) {                        //Vai buscar todos os algarismos superiores a 0 e guarda-os no buffer   
                buffer[i] = (numero % 10) + '0';
                numero = numero / 10;
                i++;
        }

        int j;
        for (j = 0; j < i; j++) {                               //o ciclo for vai imprimir todos os numeros pela ordem correta
            video[(posicao + j) * 2] = buffer[i - 1 - j];
            video[(posicao + j) * 2 + 1] = 0x0F;
        }
    }
    
