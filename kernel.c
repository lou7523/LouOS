extern void keyboard_handler();
extern unsigned char tss_descriptor[];
extern void saltarRing3(unsigned int eip);
extern void syscallHandler();
void pic_init();
void idt_init();
void idt_set(int numero, unsigned int handler);
void idtSetType(int numero, unsigned int handler, unsigned char tipo);
void imprimirNumero(int numero, int posicao);
void scroll();
void lerMapaMemoria();
void inicializarMemoria();
void configurarTSS();
void handlerGPF();
void pageFault();
void doubleFault();
void syscallHandlerC();
unsigned int alocarPagina();
unsigned int lerFicheiro(char* nome, unsigned char* destino);   //le um ficheiro do FAT32 (por nome 8.3) para "destino", devolve o tamanho ou 0 se nao encontrar
void libertarPagina(unsigned int endereco);
int verificarColisao(unsigned int endA, unsigned int tamA, unsigned int endB, unsigned int tamB);
typedef unsigned int entradaPagina;
entradaPagina pageDirectory[1024] __attribute__((aligned(4096)));
entradaPagina pageTables[2][1024] __attribute__((aligned(4096)));   //so 2 Page Tables (8MB mapeados) em vez de 1024 - as outras 1022 nunca eram usadas e inchavam o kernel.bin em ~4MB (ficava tudo no .bss, e o "ld --oformat binary" escreve o .bss todo como zeros no binario)


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

struct TSS {
    //originalmente o Ring 1 e 2 tinham uso
    //Ring 1 - drivers de dispositivos 
    //Ring 2 - servicos de sistema

unsigned int prev_tss;                  //em sistemas de multitasking este e usado para guardar o TSS para ser possivel para voltar a tras
    unsigned int esp0;                  //(esp0 - topo do stack) quando um programa ring 3faz um sys call, a CPU carrega automaticamente estes valorespara mudar para astack do kernel 
    unsigned int ss0;                   //(ss0 - seletor do segmento em 0x10 dados Ring 0)
    unsigned int esp1;                  //stacks do ring 1 e 2 (nao se usa)
    unsigned int ss1;                   //stacks do ring 1 e 2
    unsigned int esp2;                  //stacks do ring 1 e 2
    unsigned int ss2;                   //stacks do ring 1 e 2
    unsigned int cr3;                   //Page Directory Base - Em multitasking por hardware, cada tarefa tem o seu espaco de memoria virtual
    unsigned int eip;                   //Instruction Pointers - Guarda o endereco da proxima instrucao no TSS pelo CPU
    unsigned int eflags;                //Instruction Flags - Guarda o endereco da proxima instrucao na CPU
    unsigned int eax;                   //Registos gerais, a CPU guardava aqui os registos de uma tarefa quando a suspendia
    unsigned int ecx;                   //Registos gerais, a CPU guardava aqui os registos de uma tarefa quando a suspendia
    unsigned int edx;                   //Registos gerais, a CPU guardava aqui os registos de uma tarefa quando a suspendia
    unsigned int ebx;                   //Registos gerais, a CPU guardava aqui os registos de uma tarefa quando a suspendia
    unsigned int esp;                   //Registos gerais, a CPU guardava aqui os registos de uma tarefa quando a suspendia
    unsigned int ebp;                   //Registos gerais, a CPU guardava aqui os registos de uma tarefa quando a suspendia
    unsigned int esi;                   //Registos gerais, a CPU guardava aqui os registos de uma tarefa quando a suspendia
    unsigned int edi;                   //Registos gerais, a CPU guardava aqui os registos de uma tarefa quando a suspendia
    unsigned int es;                    //Registos de Segmento - os seletores de segmento de tarefa em multitasking por hardware
    unsigned int cs;                    //Registos de Segmento - os seletores de segmento de tarefa em multitasking por hardware
    unsigned int ss;                    //Registos de Segmento - os seletores de segmento de tarefa em multitasking por hardware
    unsigned int ds;                    //Registos de Segmento - os seletores de segmento de tarefa em multitasking por hardware
    unsigned int fs;                    //Registos de Segmento - os seletores de segmento de tarefa em multitasking por hardware
    unsigned int gs;                    //Registos de Segmento - os seletores de segmento de tarefa em multitasking por hardware
    unsigned int ldt;                   //Local Decriptor Table - uma tarefa de descritores privada a cada tarefa
    unsigned short trap;                //Trap Flag - se o bit 0 estiver a 1, o CPU lanca uma exepcao de debug antes de executar qualque intrucao
    unsigned short iomap_base;          //I/O permision bitmap base - offset dentro do TSS onde comecao um mapa de bits que controla quais portos de I/O o programa em Ring 3 pode aceder diretamente

    /*
        O que vou usar (por enquanto) -> esp0 (stack do kernel para a Ring 3 -> Ring 0)
                                      -> ss0 (seletor do segmento da stack do kernel)
                                      -> iomap_base (bloquear acessoo a portos em Ring 3)
    */
} __attribute__ ((packed));

struct TSS tss;

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

unsigned int carregarElf(unsigned char* elf) {
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

    return header->entryPoint;                  //devolvo o endereco

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
    unsigned char buffer[65536];   //64KB, espaco suficiente para o PROGRAMA.ELF inteiro (antes lia-se so 16 sectores fixos, 8KB, direto do disco)

    unsigned int tamanho = lerFicheiro("PROGRAMAELF", buffer);   //vai procurar e copiar PROGRAMA.ELF do FAT32 para o buffer

    if (tamanho == 0) {                 //ficheiro nao encontrado no root directory
        char* video = (char*) 0xB8000;
        video[0] = '?';                 //escreve um '?' vermelho no canto superior esquerdo do ecra como indicador de erro
        video[1] = 0x4F;
        return;
    }

    unsigned int entryPoint = carregarElf(buffer);
    saltarRing3(entryPoint);
}

    void inicializarPaginacao() {
        for (int i = 0; i < 1024; i++) {
            pageDirectory[i] = 0;           //loop for que limpa todas as entradas do PageDirectpry
    }
        for (int j = 0; j < 1024; j++) {                    //preenche as duas primeiras Page Tables (8MB)
            pageTables[0][j] = (j * 4096) | 0x07;           //mapeia as paginas de 0-1023
            pageTables[1][j] = ((1024 + j) * 4096) | 0x07;  //mapeia as paginas de 1024-2047

            //0x07 -> Este vai adicionar flags: bit 0 = 1 -> presente; bit 1 = 1 -> writable; bit 2 = 1 -> acessivel em Ring 3
        }

        pageDirectory[0] = (unsigned int) pageTables[0] | 0x07; //Liga as duas PageTables ao Page Directory
        pageDirectory[1] = (unsigned int) pageTables[1] | 0x07;

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
    idt_set(8, (unsigned int) doubleFault);
    idt_set(13, (unsigned int) handlerGPF);
    idt_set(14, (unsigned int) pageFault);
    idtSetType(0x80, (unsigned int) syscallHandler, 0xEE);
    configurarTSS();
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

void idtSetType(int numero, unsigned int handler, unsigned char tipo) {
    idt[numero].enderecoBaixo = handler & 0xFFFF;
    idt[numero].selector = 0x08;
    idt[numero].zero = 0;
    idt[numero].tipo = tipo;
    idt[numero].enderecoAlto = (handler >> 16) & 0xFFFF;
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

    /*
        Introducao ao Rings

        O que sao? 
        A CPU tem 4 niveis de privilegio, chamados de rings

        Ring 0 -> kernel (acesso total ao hardware, memoria....)
        Ring 1 -> nao usado em sistemas modernos
        Ring 2 -> nao usado em sistemas modernos
        Ring 3 -> User Mode (acesso restrito, nao toca no hardware)

        Atualmente todos os programas ocorrem no Ring 0 (nivel mais priviligiado)
        Um programa com bug pode corromper o kernel, aceder ao hardware diretamente
        ou fazer o sistema parar.

        O que muda com o Ring 3

        O programa nao vai poder executar instrucoes priviligiadas (cli, sti, lgdt...)
        Nao pode aceder ao portos de hardware (outb, inb)
        Nao pode modificar as tabelas de paginas
        Pode aceder a memoria que o kernel lhe deu
        Pode fazer system calls para pedir coisas ao kernel

        Se o programa tentar fazer algo proibido, o CPU lanca uma excecao e o kernel termina o programa

        O que e preciso para implementar Ring 3

        TSS (Task State Segment), estrutura que a CPU usa para mudar de Ring 3 para Rinf 0
        Entradas na GDT para segmentos de Ring 3
        Saltar para Ring 3 (instrucao especial)

        TSS para saltar para o Ring 3, a CPU precisa de saber para onde voltar quando o programa fizer 
        uma System Call ou der uma excepcao, ou seja, precisa de saber qual e a stack so kernel (Ring 0)

        Essa info esta na TSS, esta tem muitos campos, mas so vai ser preciso preencher 2
            ss0 -> seletor do segmento de stack de Ring 0 (0x10)
           esp0 -> endereco do topo da stack de Ring 0
    */

    void configurarTSS() {
        //Meter o TSS a zeros
        unsigned char* p = (unsigned char*) &tss;
        for (int i = 0; i < sizeof(tss); i++) {
            p[i] = 0;
        }

        //preencher os campos que vou usar
        tss.ss0 = 0x10;                     //seletor de segmento de dados Ring 0
        tss.esp0 = 0x90000;                 //topo da stack do kernel
        tss.iomap_base = sizeof(tss);       //bloquear acesso a portos em Ring 3

        unsigned int base = (unsigned int) &tss;
        tss_descriptor[2] = base & 0xFF;
        tss_descriptor[3] = (base >> 8) & 0xFF;
        tss_descriptor[4] = (base >> 16) & 0xFF;
        tss_descriptor[7] = (base >> 24) & 0xFF;

        __asm__ volatile ("ltr %0" : : "r"((unsigned short) 0x28));

    }

    void handlerGPF() {
        char* video = (char*) 0xB8000;
        char* msg = "ERRO: general protection fault";
        int i = 0;
        while (msg[i]) {
            video[i * 2] = msg[i];
            video[i * 2 + 1] = 0x4F;
            i++;
        }
        while (1);
    }

    void pageFault() {
        char* video = (char*) 0xB8000;
        char* msg = "ERRO: page fault";
        int i = 0;
        while (msg[i]) {
            video[i * 2] = msg[i];
            video[i * 2 + 1] = 0x4F;
            i++;
        }
        while(1);

    }

    void doubleFault() {
        char* video = (char*) 0xB8000;
        char* msg = "ERRO: double fault";
        int i = 0;
        while (msg[i]) {
            video[i * 2] = msg[i];
            video[i * 2 + 1] = 0x4F;
            i++;
        }
        while(1);

    }

    void syscallHandlerC() {
        unsigned int syscallNum;
        __asm__ volatile ("mov %%eax, %0" : "=r"(syscallNum));

        if (syscallNum == 1) {
            char* video = (char*) 0xB8000;
            char* msg = "Syscall";
            int i = 0;
            while (msg[i]) {
                video[i * 2] = msg[i];
                video[i * 2 + 1] = 0x4F;
                i++;
            }
        }
    }

    //BootSector

    struct FAT32BootSector {
        unsigned char jump[3];              //Os primeiros 3 bytes do sector sao uma instrucao x86 (JMP + NOP), 
                                            //este salta por cima dos dados do bootsector para o codigo de arranque
                                            //Como ja tenho bootloader nao vou usar isto (em principio)
        unsigned char oemName[8];           //Nome do sistema que formatou o disco
        unsigned short bytesPerSector;      //Quase sempre 512. Este define o tamanho de cada sector fisico do disco
                                            //usa-se este valor para calcular onde estao as estruturas do filesystems
        unsigned char sectorsPerCluster;    //Normalmente este e 8 (4KB p/ cluster) Um ficheiro de 1 byte ocupa um cluster inteiro
        unsigned short reservedSectors;     //Quantos sectores antes do primeiro ficheiro FAT
        unsigned char numFAT;               //(2) Há sempre um ficheiro FAT principal e a sua copia de seguranca
                                            //se o principal estiver corrompido o OS pode usar a copia de seguranca
        unsigned short rootEntryCount;      //Em FAT32 e sempre 0. Em FAT12 e 16 estes tem um valor fixo
        unsigned short totalSectors16;      //Em FAT32 este é sempre 0. Era usado em FAT12/FAT16 para discos pequenos (16-bits)
        unsigned char mediaType;            //0xF8 para o disco rigido fixo, 0xF0 para disquetes. Nao vai ser usado
        unsigned short sectoresPerFAT16;    //Em FAT32 e sempre 0, mas este era usado em FAT12/16, nao vamos usar aqui
        unsigned short sectoresPerTrack;    //Sectores por trilha, geometria fisica do disco (CHS), nao e importante pois estamos a usar LBA
        unsigned short numHeads;            //Tambem nao é relevante pois usamos LBA
        unsigned int hiddenSectors;         //Sectores ocultos quantos sectores existem antes da particao do disco
        unsigned int totalSectors32;        //Numero total de sectores do volume. Usa-se para saber o tamanho total do disco
        unsigned int sectorsPerFAT32;      //Quantos sectores ocupa cada copia de FAT 
                                            //inicio da area de dados = reservedSectors + (numFATs * sectorsPerFAT32)   
        unsigned short extFlags;            //Controla qual FAT esta ativa (main / copy)
        unsigned short fsVersion;           //Versao do Filesystem (0x0000) normalmente 0.0
        unsigned int rootCluster;           //Cluster do root directory, este e normalmente 2, e aqui que comeca a lista de ficheiros e pastas da raiz
        unsigned short fsInfo;              //Normalmente e 1. O fsInfo é uma estrutura que guarda o numero de clusters livres para nao ter de contar sempre 0
        unsigned short backupBootSector;    //Sector de backup do bootsector, normalmente 6, copia do boot sector para a recuperacao em caso de corrupcao
        unsigned char reserved[12];         //Bytes a 0 para o futuro
        unsigned char driveNumber;          //0x80 para hard drive, 0x00 para disquetes. Legado da BIOS (ignorar)
        unsigned char reserved2;            //Sempre 0 (ignorar)
        unsigned char bootSignature;        //0x29 indica o volumeID, volumeLabel, fsType
        unsigned int volumeID;              //Numero aleatorio gerado quando o disco foi formatado. Usado para identificar o disco.
        unsigned char volumeLabel[11];      //O nome que se da ao disco
        unsigned char fsType[8];            //Usado para detectar campos anteriores
        /*
            Disto tudo só se vai usar:
                -bytesPerSector 
                -sectorPerCluster
                -reservedSectors
                -numFATs
                -sectorsPerFAT32
                -rootCluster

            Tudo o resto e legado, cosmetico ou para compatibilidade
        */
    } __attribute__((packed));

    struct FAT32DirEntry {
        unsigned char nome[8];          //Nome do ficheiro
        unsigned char ext[3];           //extensao (.txt .pdf .jpg)
        unsigned char atributos;        //tipo de entrada (ficheiro, pasta)
        unsigned char ignorados1[8];    //campos nao usados
        unsigned short clusterAlto;     //16 bits superiores do cluster
        unsigned char ignorado2[4];     //campos nao usados
        unsigned short clusterBaixo;    //16 bits inferiores do cluster
        unsigned int tamanho;           //tamanho do ficheiro em bytes
        //Cluster - É um grupo de entradas de sectores consecutivos no disco, é a unidade minima de alocacao de ficheiros FAT32

    } __attribute__ ((packed)); 

    unsigned int lerFicheiro(char* nome, unsigned char* destino) {
        //Le um ficheiro do root directory do FAT32 pelo nome 8.3 (ex: "PROGRAMAELF", sem ponto nem espacos)
        //e copia todo o seu conteudo, cluster a cluster, para "destino". Devolve o tamanho do ficheiro em bytes, ou 0 se nao encontrar.

        unsigned char buffer[512];
        lerSector(0, buffer);                  //le o boot sector (sector 0) para conseguir os parametros do filesystem

        struct FAT32BootSector* bs = (struct FAT32BootSector*) buffer;

        unsigned int fatStart = bs->reservedSectors;                                      //1o sector da FAT
        unsigned int dataStart = bs->reservedSectors + bs->numFAT * bs->sectorsPerFAT32;   //1o sector da area de dados (depois das copias de FAT)
        unsigned int sectorsPerCluster = bs->sectorsPerCluster;                            //guardado numa variavel local - "buffer" (e por isso "bs") vai ser reescrito a seguir com o root directory,
                                                                                            //e sem isto o codigo mais abaixo ia ler este campo de bytes de uma entrada de diretorio em vez do boot sector

        unsigned int sectorRaiz = dataStart + (bs->rootCluster - 2) * sectorsPerCluster;  //sector do cluster 2 (root directory) - o -2 e porque os clusters 0 e 1 sao reservados
        lerSector(sectorRaiz, buffer);          //le o 1o sector do root directory (16 entradas de 32 bytes, ja que buffer tem 512 bytes)
                                                 //isto sobrescreve o boot sector no buffer - dai termos guardado sectorsPerCluster antes

        struct FAT32DirEntry* dir = (struct FAT32DirEntry*) buffer;

        for(int i = 0; i < 16; i++) {           //percorre as 16 entradas do sector a comparar com o nome pedido
            int encontrou = 1;
            for (int j = 0; j < 11; j++) {      //compara byte a byte os 11 bytes do formato 8.3 (8 do nome + 3 da extensao)
                unsigned char byteDir;
                if (j < 8) {
                    byteDir = dir[i].nome[j];
                } else {
                    byteDir = dir[i].ext[j - 8];
                }
                if (byteDir != nome[j]) {
                    encontrou = 0;
                    break;
                }
            }

            if (encontrou) {
                unsigned int cluster = ((unsigned int) dir[i].clusterAlto << 16) | dir[i].clusterBaixo;  //junta os 16 bits altos e baixos no cluster inicial do ficheiro
                unsigned int tamanho = dir[i].tamanho;

                unsigned int bytesCopiados = 0;

                while (cluster < 0x0FFFFFF8) {     //0x0FFFFFF8 e mais e o marcador de fim de cadeia (EOF) na FAT32
                    unsigned int sector = dataStart + (cluster - 2) * sectorsPerCluster;   //sector absoluto de disco onde comeca este cluster

                    for (int x = 0; x < sectorsPerCluster; x++) {      //copia todos os sectores do cluster para o destino, sector a sector
                        lerSector(sector + x, destino + bytesCopiados);
                        bytesCopiados += 512;
                    }

                    unsigned char fatBuffer[512];
                    unsigned int fatSector = fatStart + (cluster * 4) / 512;   //cada entrada da FAT32 ocupa 4 bytes, por isso *4 para obter o offset em bytes e /512 para saber o sector
                    unsigned int fatOffset = (cluster * 4) % 512;              //offset dentro desse sector onde esta a entrada
                    lerSector(fatSector, fatBuffer);
                    cluster =*(unsigned int*)(fatBuffer + fatOffset) & 0x0FFFFFFF;    //le o proximo cluster da cadeia (mascara os 4 bits superiores, reservados)
                }

                return tamanho;
            }
        }

        return 0;      //nao encontrou nenhuma entrada com o nome pedido
    }