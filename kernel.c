extern void keyboard_handler();
void pic_init();
void idt_init();
void idt_set(int numero, unsigned int handler);
void imprimirNumero(int numero, int posicao);
void scroll();
void lerMapaMemoria();
void inicializarMemoria();
unsigned int alocarPagina();
void libertarPagina(unsigned int endereco);
int verificarColisao(unsigned int endA, unsigned int tamA, unsigned int endB, unsigned int tamB);
typedef unsigned int entradaPagina;
entradaPagina pageDirectory[1024] __attribute__((aligned(4096)));
entradaPagina pageTables[1024][1024] __attribute__((aligned(4096)));



#define TamanhoPagina 4096          //4KB, pois este e o valor definido pela arquitetura x86
#define TotalPagina 32768           /*
                                        RAM que necessita de ser suportada: 128MB
                                        128MB em bytes: 128 * 1024 * 1024 = 134217728
                                        Dividido pelo tamanho de cada pagina: 134217728 / 4096 = 32768, este numero e o maximo de paginas possiveis
                                    */
unsigned int bitmap[1024];

struct blocoMemoria {
    unsigned long long enderecoBase;
    unsigned long long tamanho;
    unsigned int tipo;
} __attribute__((packed));

struct elfHeader {                      //cabecalho no inicio de ficheiros ELF
    unsigned char magico[4];            //4 numeros magicos para ver se o ficheiro ELF realmente o é, e para ver se funciona
    unsigned char classe;               //1 = 32 bits / 2 = 64 bits, vamos verificar se e 1
    unsigned char dados;                //little-endian -> no byte menos significativo para o mais
    unsigned char versao;               //Campos que nao uso
    unsigned char padding[9];           //o padding sao bytes reservados que a especificacao ELF deixou para uso futuro. Sempre 0
    unsigned short tipo;                //vamos verificar se tipo = 2, se sim vai ser executavel
    unsigned short maquina;             //3 =x86 32bits. Vamo verificar se e 3 para confirmar se o programa tem a arquitetura certa
    unsigned int versao2;               //versao do formato ELF, sempre 1
    unsigned int entryPoint;            //Endereco de memorio para onde o CPU deve ir para executar o programa (0x400024)
    unsigned int phOffset;              //le quantos bytes desde o inicio do ficheiro estao os Program Headders
    unsigned int shOffset;              //Nao sao precisas para executar o programa, so para debugging
    unsigned int flags;                 //especifico da arquitetura, normalmente 0
    unsigned short ehSize;              //tamanho deste header (52 bytes)
    unsigned short phEntSize;           //tamanho de cada Programm Header (32 bytes)
    unsigned short phNum;               //Quantos programm headers existem
    unsigned short shEntSize;           //campos de section headers
    unsigned short shNum;               // ""       ""        ""
    unsigned short shStrIndex;          // ""       ""        ""
} __attribute__ ((packed));

struct programHeader {                  //estrutura do program header
    unsigned int tipo;                  //1 = PT_LOAD
    unsigned int offset;                //Onde esta o segmento do ficheiro
    unsigned int vaddr;                 //para que endereco de memoria copiar este segmento
    unsigned int paddr;                 //versao fisica do vaddr
    unsigned int filesz;                //Quantos bytes este ocupa no ficheiro
    unsigned int memsz;                 //Quantos bytes este ocupa na memoria
    unsigned int flags;                 //Permissoes do Segmento R-read; W-write; E-execution;
    unsigned int align;                 //Alinhamento do segmento de memoria
} __attribute__ ((packed));

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

static inline unsigned short inw(unsigned short porto) {
    unsigned short resultado;
    __asm__ volatile ("inw %1, %0" : "=a"(resultado) : "Nd"(porto));
    return resultado;
}

static inline unsigned char inb(unsigned short porto) {
    unsigned char resultado;
    __asm__ volatile ("inb %1, %0" : "=a"(resultado) : "Nd"(porto));
    return resultado;
}

int carregarElf(unsigned char* elf) {
    struct elfHeader* header = (struct elfHeader*) elf; //elf e um ponteiro para bytes,e em vez de ler um a um manualmente convertemos para elfHeader
                                                        //para o codigo ser mais facil de manter e mudar
    if (header->magico[0] != 0x7F || header->magico[1] != 'E' || header->magico[2] != 'L' || header->magico[3] != 'F') {    //Lei de Morgan
        return 0;                                                                                                           //Condicao que verifica se o ficheiro e valido
    }

    if (header->classe != 1) return 0;          /*
                                                    se for 64 bits, os offsets e tamanhs das structs sao diferentes
                                                    elfHeader e programHeader tem layouts diferentes em 64bit
                                                    tentar le los com as structs de 32 bits daria valores completamente errados
                                                */
    if (header->tipo != 2) return 0;            /*
                                                    podia ser da biblioteca .so ou ficheiros de objetos .o e esses nao tem entry point
                                                    executavel, se a CPU fosse para la matava o sistema    
                                                */

    struct programHeader* ph = (struct programHeader*) (elf + header->phOffset);
    /*
        '->' operador de acesso membro através do ponteiro
        fazemos essa struct acima para calcular os offsets automaticamente
        pois se nao tinhamos de calcular um a um
    */

    for (int i = 0;header->phNum > i; i++) {
         if (ph[i].tipo == 1) {
            unsigned char* origem = elf + ph[i].offset;
            unsigned char* destino = (unsigned char*) ph[i].vaddr;
            int j;
            for (j = 0; j < ph[i].filesz; j++) {
                destino[j] = origem[j];
            }

            for (j = ph[i].filesz; j < ph[i].memsz; j++) {
                destino[j] = 0;
            }
        }
    }

    void (*entrada)() = (void (*)()) header->entryPoint;    /*
                                                                header->entryPoint:
                                                                    - Numero onde o endereco de memoria comeca. No programa.elf é
                                                                      0x400024.
                                                                    
                                                                (void (*)()):
                                                                    - É um cast que converte o numero desse ponteiro para uma funcao

                                                                void (*entrada)():
                                                                    - Declara uma variavel chamada entrada que é um ponteiro para uma 
                                                                      funcao.

                                                                    entrada():
                                                                    - Chama a funcao que está no endereco guardado em entrada, ou seja,
                                                                      vai para 0x400024 e comeca a executar o programa
                                                            */
    entrada();

    return 1;

}

void lerSector(unsigned int lba, unsigned char* destino) {
    outb(0x1F2, 1);                         //le o setor 1
    outb(0x1F3, lba & 0xFF);                //bits 0-7
    outb(0x1F4, (lba >> 8) & 0xFF);         //bits 8-15
    outb(0x1F5, (lba >> 16) & 0xFF);        //bits 16-23
    outb(0x1F6, 0xE0 | (lba >> 24));        //bits 24-27 + modo LBA
    outb(0x1F7, 0x20);                      //comando ler

    //BSY - Busy, este e obit 7 do porto 0x1F7. Quando o IDE cmc a funcionar, vai buscar dados ao disco, dirante esse tempo BSY = 1 (on) e so quando termina fica BSY = 0
    //DRQ - Data Request, este e o 3 bit do porto 0x1F7 e quando o BSY = 0, o DRQ = 1 "os dados estao no buffer, podes le los por agora"
    //LBA - Logical Block Address, numero do sector que quero ler. (LBA cmc sempre no 0)

    //espera o controlar estar pronto
    while (inb(0x1F7) & 0x80);          //espera BSY = 0
    while (!(inb(0x1F7) & 0x08));       //espera DRQ = 1

    //ler 256 words do porto 0x1F0 para o destino
    for (int i = 0; i < 256; i++) {
        ((unsigned short*) destino)[i] = inw(0x1F0);
    }
}

void executarPrograma() {
    unsigned char buffer[8192];
    for (int i = 0; i < 16; i++) {
        lerSector(22 + i, buffer + i * 512);
    }

    carregarElf(buffer);
}

    void inicializarPaginacao() {
        for (int i = 0; i < 1024; i++) {
            pageDirectory[i] = 0;           //loop for que limpa todas as entradas do PageDirectpry
    }
        for (int j = 0; j < 1024; j++) {                    //preenche as duas primeiras Page Tables (8MB)
            pageTables[0][j] = (j * 4096) | 0x03;           //mapeia as paginas de 0-1023
            pageTables[1][j] = ((1024 + j) * 4096) | 0x03;  //mapeia as paginas de 1024-2047

            //0x03 -> Este vai adicionar flags: bit 0 = 1 -> presente; bit 1 = 1 -> writable
        }

        pageDirectory[0] = (unsigned int) pageTables[0] | 0x03; //Liga as duas PageTables ao Page Directory
        pageDirectory[1] = (unsigned int) pageTables[1] | 0x03;

        __asm__ volatile ("mov %0, %%cr3" : : "r"(pageDirectory));  //Carrega o endereco do cr3, o registo especial da CPU que guarda onde esta o page directory.
                                                                    //Sem isto o CPU nao sabe onde procurar tabelas
        unsigned int cr0;                                           //le o valor de cr0
        __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));                         
        cr0 |= 0x80000000;                                          //Ativa o bit o bit 31 (PG - PageEnable)
        __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));            //Escreve de volta

        /*
            A partir deste momento, todos os enderecos que o CPU usa sao tratados como virtuais e traduzidos atraves
            das tabelas construidas
        */
    } 


void desligarCursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);       //bit 5 a 1 desliga o cursor de hardware
}

void kernel_main() {
    desligarCursor();

    char* video = (char*) 0xB8000;
    int i;
    for (i = 0; i < 80 * 25 * 2; i += 2) {
        video[i] = ' ';
        video[i + 1] = 0x0F;
    }

    unsigned int enderecoBitmap = (unsigned int) bitmap;
    unsigned int tamanhoBitmap = 1024 * 4;
    if (verificarColisao(0x2000, 20 * 32, enderecoBitmap, tamanhoBitmap)) {    //0x2000 -> endereco do mapa de memoria, 20 -> tamanho de cada bloco, 32 -> n maximo de blocos que o ciclo int 0x15 pode escrever
        video[0] = 'C';                                                         //onde comeca                            tamanho total reservado para o bitmap (640bytes)
        video[1] = 0x4F;    
        while(1);
    }

    inicializarMemoria();
    inicializarPaginacao();
    lerMapaMemoria();

    pic_init();
    idt_init();
    idt_set(0x21, (unsigned int) keyboard_handler);

    __asm__ volatile ("sti"); //liga interrupçoes 
    //sem isto o CPU vai ignorar o PIC, mesmo com o IDT inicializado corretamente

    executarPrograma();

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


    void lerMapaMemoria() {
        struct blocoMemoria* mapa = (struct blocoMemoria*) 0x2000;

        int i = 0;
        while(mapa[i].tipo != 0) {
            imprimirNumero(mapa[i].tipo, i * 5);
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

    void inicializarMemoria() {
        for (int i = 0; i < 1024; i++) {    //vai marcar todas as paginas como ocupadas, para nao haver risco de sem querer dar overwrite em enderecos ocupados
            bitmap[i] = 0xFFFFFFFF;         //mete 0xFFFFFFFF, ou seja, todos os 32 bits e mete-os a 1, 1 = todas as paginas ocupadas
        }

        struct blocoMemoria* mapa = (struct blocoMemoria*) 0x2000;      //Percorrer o mapa da BIOS que escreveu em 0x2000

        int j = 0;
        while (mapa[j].tipo != 0) {                                                             //0 nao escrito, fim da lista
            if (mapa[j].tipo == 1) {                                                            //1 -> livre; 2 -> reservado

                //calcular qual e a primeira pagina deste bloco
                //ex: enderecoBase 0x100000 (1MB) / 4096 = pagina 256
                unsigned int primeiraPagina = mapa[j].enderecoBase / TamanhoPagina;

                //calcular quantas paginas cabem neste bloco
                //ex: tamanho 0x7EE0000 (127MB) / 4096 = 32480 paginas
                unsigned int numPaginas = mapa[j].tamanho / TamanhoPagina;

                //marcar cada pagina deste bloco como livre (bit a 0)
                for (int x = 0; x < numPaginas; x++) {
                    //primeira pagina + x / 32 -> unsigned int do bitmap
                    //(primeiraPagina + x) % 32 -> qual bit dentro desse unsigned int
                    //~(1 << bit) -> todos os bits a 1 excepto o que queremos pôr a 0
                    //&= aplica a mascara, so alterando esse bit
                    bitmap[(primeiraPagina + x)/ 32] &= ~(1 << ((x + primeiraPagina) % 32));
                }
            }
        j++;        //vai para o seguinte
        }
    };

    unsigned int alocarPagina() {
        for (int a = 0; a < 1024; a++) {                                    //percorre os 1024 unsigned ints do bitmap
            for (int b = 0; b < 32; b++) {                                  //percorre os 32 bits de cada unsigned int
                if (!(bitmap[a] & (1 << b))) {                              //se o bit b do elemento a for 0 -> pagina livre
                    bitmap[a] |= (1 << b);                                  //se tiver livre marca a pagina como ocupada
                    unsigned int endereco = (a * 32 + b) * TamanhoPagina;    /*
                                                                                Formula para saber o endereco da pagina

                                                                                a -> numero do unsigned int que parou
                                                                                b -> numero do bit do unsigned int de a
                                                                                32 -> pois cada unsigned int tem 32 bits
                                                                                Tamanho da Pagina -> 4096;

                                                                                EX:
                                                                                    a = 256
                                                                                    b = 0

                                                                                    (256 * 32 + 0) = 8192
                                                                                    endereco = 8192 * 4096 = 33554432
                                                                                    33554432 / 1024 / 1024 = 32MB

                                                                                    32MB -> 0x2000000

                                                                                Ao saber o tamanho da pagina conseguimos saber o endereco
                                                                            */
                    return endereco;                                         //vai devolver a variavel endereco
                }
            }
        }
        return 0;                                                           //se estiver ocupada retorna 0

    };

    void libertarPagina(unsigned int endereco) {
        unsigned int pagina = endereco /TamanhoPagina;
        bitmap[pagina / 32] &= ~(1 << (pagina % 32));
    }

    //Detector de Colisoes de Memoria
    int verificarColisao(unsigned int endA, unsigned int tamA, unsigned int endB, unsigned int tamB) {
        if (endA + tamA <= endB) return 0;
        if (endB + tamB <= endA) return 0;

        return 1;
    }