static incline void outb(unsigned short porto, unsigned char valor) {
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
} __atributo__((packed));   //impede a CPU de ler lixo

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
    idt[numero].tipo = handler & 0xFFFF;
    idt[numero].enderecoAlto = (handler >> 16) & 0xFFFF;    //desloca o endereco 16 bits para a direita, ficando com os ultimos 16bits
}

void idt_init() {
    idt_desc.tamanho = sizeof(idt) - 1;
    idt_desc.endereco = (unsigned int) idt;


    __asm__ volatile ("lidt %0" : : "m"(idt_desc));

    //lidt - instrucao em assembly que conecta o decriptor da IDT na GPU
}

    __asm__ volatile ("sti"); //liga interrupçoes 
    //sem isto o CPU vai ignorar o PIC, mesmo com o IDT inicializado corretamente