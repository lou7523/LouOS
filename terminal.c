#include "terminal.h"

int termX = 10;                 //Coordenadas do cursor
int termY = 60;                 //Coordenadas do cursor
char linhaAtual[256];
int linhaPos = 0;
int errorBoxVisivel = 0;        //1 enquanto a caixa de erro esta desenhada no ecra
int errorBoxAltura = 100;       //altura da caixa atualmente desenhada (varia consoante o comando)

unsigned char corTextoR = 0, corTextoG = 255, corTextoB = 0;    //Cor texto vermelhar
unsigned char corFundoR = 0, corFundoG = 0, corFundoB = 0;      //Cor fundo preta

#define ERROR_BOX_X 150
#define ERROR_BOX_Y 250
#define ERROR_BOX_W 500
#define ERROR_BOX_H 100
#define border 10

static inline void outb(unsigned short porto, unsigned char valor) {    
    __asm__ volatile ("outb %1, %0" : : "Nd"(porto), "a"(valor));

    /*
        static inline - diz ao compilador para copiar esta funcao diretamente para onde e chamada
        outb - instrucao x86 que escreve um byte num porto do hardware
        Nd - o porto vai diretamente para a instrucao
        a - o valor vai para o registo AL
    */
}

void iniciarTerminal() {
    preencherEcra(corFundoR, corFundoG, corFundoB);                                     //Preenche o ecra a preto, "limpa o ecra"
    desenharTexto(10, 10, "LouOS Terminal", corTextoR, corTextoG, corTextoB, 2);      //Escreve LouOS no topo da janela do terminal
    desenharTexto(10, 40, "> ", corTextoR, corTextoG, corTextoB, 2);                  //Escreve '> ' para o user escrever em frente
    termX = 10 + 2 * 8 * 2;                                     //Coluna de inicio
    termY = 40;                                                 //Linha de Inicio
}

void terminalHandleChar(char c) {
    if (errorBoxVisivel) {                                      //o user comecou a escrever: apaga a caixa de erro
        desenharJanelas(ERROR_BOX_X, ERROR_BOX_Y, ERROR_BOX_W, errorBoxAltura, corFundoR, corFundoG, corFundoB);
        errorBoxVisivel = 0;
    }
    if (c == '\n') {                                          // Enter premido: avanca 20px (nova linha) e repoe ">"
        int resultado = compararStrings();
        linhaAtual[linhaPos] = 0;
        linhaPos = 0;
        if (resultado != 2 && resultado != 3) {                   //resultado 2 = "clear", 3 = "quit", ambos ja tratam do ecra e do prompt
            termY += 20;
            if (chegouAoFimDaJanela()) {                            //nao ha mais espaco, "scroll": limpa o ecra e recomeca do topo
                clear();
            } else {
                termX = 10;
                desenharTexto(10, termY, "> ", corTextoR, corTextoG, corTextoB, 2);           //para desenhar ">"
                termX = 10 + 2 * 8 * 2;                                 //Calcula a prox posicao ">"
            }
        }
    } else if (c == '\b') {
        if (linhaPos > 0) {
            linhaPos--;
            termX -= 8 * 2;
            desenharJanelas(termX, termY, 8 * 2, 8 * 2, corFundoR, corFundoG, corFundoB);
        }
    } else {
        if (linhaPos < 255) {
            if (termX + 8 * 2 > 800) {                              //nao cabe mais nenhum caracter nesta linha, quebra ANTES de desenhar
                termY += 20;                                       //(se fosse depois, o glifo era desenhado a transbordar dos 800px
                if (chegouAoFimDaJanela()) {                        //tambem chegou ao fundo do ecra: "scroll", limpa e recomeca
                    clear();
                } else {
                    termX = 10;                                     // e os pixeis a mais "sangravam" para a linha seguinte do framebuffer)
                }
            }
            linhaAtual[linhaPos] = c;
            linhaPos++;
            desenharChar(termX, termY, c, corTextoR, corTextoG, corTextoB, 2);
            termX += 8 * 2;
        }
    }
}

int chegouAoFimDaJanela() {
    return termY + 8 * 2 > 600;                                 //nao cabe mais nenhuma linha antes do fundo do ecra (600px)
}

int compararStrings() {
    if (igualA("clear")) {
        clear();
        return 2;
    } else if (igualA("version")) {
        version();
        return 1;
    } else if (igualA("uptime")) {
        uptime();
        return 1;
        
    } else if (igualA("mem")) {
        mostrarMemory();
        return 1;
    } else if (igualA("quit")) {
        quit();
        return 3;
    } else if (comecaCom("echo ")) {
        echo();
        return 1;
    } else if (igualA("ps")) {
        ps();
        return 1;
    } else if (comecaCom("txtcolor red")) {
        corTextoR = 255;
        corTextoG = 0;
        corTextoB = 0;
        clear();
        return 2;
    } else if (comecaCom("txtcolor green")){
        corTextoR = 0;
        corTextoG = 255;
        corTextoB = 0;
        clear();
        return 2;
    } else if (comecaCom("txtcolor blue")) {
        corTextoR = 0;
        corTextoG = 0;
        corTextoB = 255;
        clear();
        return 2;
    } else if (comecaCom("txtcolor yellow")) {
        corTextoR = 255;
        corTextoG = 255;
        corTextoB = 0;
        clear();
        return 2;
    } else if (comecaCom("txtcolor purple")) {
        corTextoR = 157;
        corTextoG = 0;
        corTextoB = 255;
        clear();
        return 2;
    } else if (comecaCom("bgcolor white")) {
        corFundoR = 255;
        corFundoG = 255;
        corFundoB = 255;
        clear();
        return 2;
    } else if (comecaCom("bgcolor black")) {
        corFundoR = 0;
        corFundoG = 0;
        corFundoB = 0;
        clear();
        return 2;
    } else if (comecaCom("bgcolor blue")) {
        corFundoR = 17;
        corFundoG = 17;
        corFundoB = 132;
        clear();
        return 2;
    } else if (igualA("reboot")){
        reboot();
        return 1;
    } else if (igualA("ls")){
        ls();
        return 1;
    } 
    else {
        errorMessage();
        return 0;
    }
}

void errorMessage() {
    errorBoxAltura = ERROR_BOX_H;
    desenharJanelas(ERROR_BOX_X, ERROR_BOX_Y, ERROR_BOX_W, ERROR_BOX_H, corTextoR, corTextoG, corTextoB);                                          //moldura verde
    desenharJanelas(ERROR_BOX_X + border, ERROR_BOX_Y + border, ERROR_BOX_W - 2 * border, ERROR_BOX_H - 2 * border, corFundoR, corFundoG, corFundoB); //interior preto

    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 20, "The command doesn't exist,", corTextoR, corTextoG, corTextoB, 2);
    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 40, "please type 'help' for help", corTextoR, corTextoG, corTextoB, 2);

    errorBoxVisivel = 1;                                //fica visivel ate o user comecar a escrever
}

void clear(){
    preencherEcra(corFundoR, corFundoG, corFundoB);
    desenharTexto(10, 10, "LouOS Terminal", corTextoR, corTextoG, corTextoB, 2);
    desenharTexto(10, 40, "> ", corTextoR, corTextoG, corTextoB, 2);
    termX = 10 + 2 * 8 * 2;
    termY = 40;
}

void version() {
    errorBoxAltura = ERROR_BOX_H;
    desenharJanelas(ERROR_BOX_X, ERROR_BOX_Y, ERROR_BOX_W, ERROR_BOX_H, corTextoR, corTextoG, corTextoB);                                          //moldura verde
    desenharJanelas(ERROR_BOX_X + border, ERROR_BOX_Y + border, ERROR_BOX_W - 2 * border, ERROR_BOX_H - 2 * border, corFundoR, corFundoG, corFundoB); //interior preto

    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 20, "LouOS", corTextoR, corTextoG, corTextoB, 2);
    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 40, "version 1.0", corTextoR, corTextoG, corTextoB, 2);

    errorBoxVisivel = 1;
}

void uptime() {
    char buffer[10];
    intParaString(contadorTicks / 100, buffer);
    errorBoxAltura = ERROR_BOX_H;
    desenharJanelas(ERROR_BOX_X, ERROR_BOX_Y, ERROR_BOX_W, ERROR_BOX_H, corTextoR, corTextoG, corTextoB);                                          //moldura verde
    desenharJanelas(ERROR_BOX_X + border, ERROR_BOX_Y + border, ERROR_BOX_W - 2 * border, ERROR_BOX_H - 2 * border, corFundoR, corFundoG, corFundoB); //interior preto

    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 20, "LouOS", corTextoR, corTextoG, corTextoB, 2);
    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 40, buffer, corTextoR, corTextoG, corTextoB, 2);

    errorBoxVisivel = 1;
}

void mostrarMemory() {
    int paginasLivre = 0;
    for (int i = 0; i < 1024; i++) {
        for (int a = 0; a < 32; a++) {
            if (!(bitmap[i] & (1 << a))) {
                paginasLivre++;    
            }
        }
    }

    paginasLivre *= 4096;
    paginasLivre /= 1048576;
    char buffer[10];
    intParaString(paginasLivre, buffer);

    errorBoxAltura = ERROR_BOX_H;
    desenharJanelas(ERROR_BOX_X, ERROR_BOX_Y, ERROR_BOX_W, ERROR_BOX_H, corTextoR, corTextoG, corTextoB);                                          //moldura verde
    desenharJanelas(ERROR_BOX_X + border, ERROR_BOX_Y + border, ERROR_BOX_W - 2 * border, ERROR_BOX_H - 2 * border, corFundoR, corFundoG, corFundoB); //interior preto

    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 20, "LouOS - MB", corTextoR, corTextoG, corTextoB, 2);
    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 40, buffer, corTextoR, corTextoG, corTextoB, 2);

    errorBoxVisivel = 1;
}

void quit() {
    termX = 10 + 2 * 8 * 2;
    termY = 40;
    preencherEcra(corFundoR, corFundoG, corFundoB);
    estadoSistema = 0;
    desenharMenu();
}

void echo() {
    errorBoxAltura = ERROR_BOX_H;
    desenharJanelas(ERROR_BOX_X, ERROR_BOX_Y, ERROR_BOX_W, ERROR_BOX_H, corTextoR, corTextoG, corTextoB);
    desenharJanelas(ERROR_BOX_X + border, ERROR_BOX_Y + border, ERROR_BOX_W - 2 * border, ERROR_BOX_H - 2 * border, corFundoR, corFundoG, corFundoB);
    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 20, &linhaAtual[5], corTextoR, corTextoG, corTextoB, 2);
    errorBoxVisivel = 1;
}

void ps() {
    char estado[30];
    errorBoxAltura = ERROR_BOX_H + 60;
    desenharJanelas(ERROR_BOX_X, ERROR_BOX_Y, ERROR_BOX_W, errorBoxAltura, corTextoR, corTextoG, corTextoB);
    desenharJanelas(ERROR_BOX_X + border, ERROR_BOX_Y + border, ERROR_BOX_W - 2 * border, errorBoxAltura - 2 * border, corFundoR, corFundoG, corFundoB);
    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 20, "Lista de Processos", corTextoR, corTextoG, corTextoB, 2);
    for (int i = 0; i < 4; i++) {
        intParaString(processos[i].estado, estado);
        desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 40 + i * 20, estado, corTextoR, corTextoG, corTextoB, 2);
    }
    errorBoxVisivel = 1;
}

void reboot() {
    outb(0x64, 0xFE);
    /*
        0x64 - porto do controlador PS/2
        0xFE - comando de reset: faz o controlador enviar um pulso LOW na linha RESET# da CPU
        o CPU interpreta esse pulso como um reset de hardware e reinicia    
    */
}

void ls() {
    unsigned char buffer[512];  //buffer de 512 bytes para ler um sector do disco (512 = tamanho fixo de um Sector FAT32)
    lerSector(0, buffer);       //le o sector 0 (boot sector) que contem os parametros do fylesystem

    struct FAT32BootSector* bs = (struct FAT32BootSector*) buffer;
    //interpreta o buffer como um boot sector FAT32 para aceder aos campos sem calcular os offsets manualmente

    unsigned int dataStart = bs->reservedSectors + bs->numFAT * bs->sectorsPerFAT32;
    /*
        calcula o 1o setor da area de dados:
        reservedSectores = sectores reservados antes da FAT
        numFAT * sectorsPerFAT32 = espaco ocupado pelas copias da FAT
        dataStart = onde comecao os dados reais (ficheiros e diretorios)
    */
    unsigned int sectorsPerCluster = bs->sectorsPerCluster;
    /*
        guarda sectorsPerCluster ANTES de reler o buffer
        a seguir o buffer cvai ser overwritten com o root directory
        e bs ficaria a apontar para dados invalidos
    */
    unsigned int sectorRaiz = dataStart + (bs->rootCluster - 2) * sectorsPerCluster;
    /*
        calcula o setor fisico do root directory
        bs->rootCluster e normalmente 2 (primeiro cluster de dados)
        o -2 e porque os clusters 0 e 1 sao reservados pelo FAT32 e nao existem na area de dados
    */
    lerSector(sectorRaiz, buffer);
    /*
        le o root directory para o buffer, overwritting o bootsector
        cada entrada do diretorio tem 32 bytes, logo 512/32 = 16 entradas por sector
    */

    struct FAT32DirEntry* dir = (struct FAT32DirEntry*) buffer;
    //interpreta o buffer como um array de entradas de diretorio (32 bytes cada)


    // conta os ficheiros primeiro
    int numFicheiros = 0;                                                   //contador de ficheiros validos encontrados
    for (int i = 0; i < 16; i++) {                                          //precorre as tais 16 entradas
        if (dir[i].nome[0] != 0 && dir[i].nome[0] != 0xE5) numFicheiros++;
        //dir[i].nome[0] == 0x00 significa que esta vazia
        //dir[i].nome[0] == 0xE5 significa entrada apagada
        //se nao for nenhum destes casos o ficheiro e valido
    }

    // desenha a caixa com altura certa
    errorBoxAltura = ERROR_BOX_H + numFicheiros * 20;
    //calcula a altura da caixa, altura da base + 20px por ficheiro

    desenharJanelas(ERROR_BOX_X, ERROR_BOX_Y, ERROR_BOX_W, errorBoxAltura, corTextoR, corTextoG, corTextoB);
    //desenha a moldura exterior na cor do texto

    desenharJanelas(ERROR_BOX_X + border, ERROR_BOX_Y + border, ERROR_BOX_W - 2 * border, errorBoxAltura - 2 * border, corFundoR, corFundoG, corFundoB);
    //desenha o interior na cor do fundo, ligeiramente menor para criar o efeito de borda

    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 20, "Ficheiros:", corTextoR, corTextoG, corTextoB, 2);
    //escreve o texto na caixa

    // lista os ficheiros
    int linha = 0;                                              //contador de linha para posicionar cada ficheiro
    for (int i = 0; i < 16; i++) {                              //percorre as tais 16 entradas
        if (dir[i].nome[0] != 0 && dir[i].nome[0] != 0xE5) {    //so processa entradas validadas
            char nome[13];                                      //buffer para nome completo: 8 nome + 1 ponto + 3 extensao + 1 null terminator
            int a;
            for (a = 0; a < 8; a++) nome[a] = dir[i].nome[a];   //copia os 8 bytes do nome
            nome[8] = '.';                                      //adiciona o ponto separador entre o nme e a extensao
            for (a = 0; a < 3; a++) nome[9 + a] = dir[i].ext[a];//copia os 3 bytes da extensao
            nome[12] = 0;                                       //termina a string com null terminator para desenharTexto saber onde parar
            desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 40 + linha * 20, nome, corTextoR, corTextoG, corTextoB, 2);
            //desenha o nome do ficheiro na linha atual (40 px no offset inicial + 20px do ficheiro anterior)
            linha++;//avanca para a proxima linha
        }
    }
    errorBoxVisivel = 1;//marcar a caixa para ser invisivel ao escrever
}

int igualA(char* comando) {
    for (int i = 0; i < linhaPos; i++) {
        if(linhaAtual[i] == comando[i]) {
    
        } else {
            return 0;
        }
    }

    return 1;
}

int comecaCom(char* prefixo) {
    int i = 0;
    while (prefixo[i] != 0) {
        if (linhaAtual[i] != prefixo[i]) return 0;
        i++;
    }
    return 1;
}