#include "terminal.h"

int termX = 10;                 //Coordenadas do cursor
int termY = 60;                 //Coordenadas do cursor
char linhaAtual[256];
int linhaPos = 0;
int errorBoxVisivel = 0;        //1 enquanto a caixa de erro esta desenhada no ecra

#define ERROR_BOX_X 150
#define ERROR_BOX_Y 250
#define ERROR_BOX_W 500
#define ERROR_BOX_H 100
#define border 10

void iniciarTerminal() {
    preencherEcra(0, 0, 0);                                     //Preenche o ecra a preto, "limpa o ecra"
    desenharTexto(10, 10, "LouOS Terminal", 0, 255, 0, 2);      //Escreve LouOS no topo da janela do terminal
    desenharTexto(10, 40, "> ", 0, 255, 0, 2);                  //Escreve '> ' para o user escrever em frente
    termX = 10 + 2 * 8 * 2;                                     //Coluna de inicio
    termY = 40;                                                 //Linha de Inicio
}

void terminalHandleChar(char c) {
    if (errorBoxVisivel) {                                      //o user comecou a escrever: apaga a caixa de erro
        desenharJanelas(ERROR_BOX_X, ERROR_BOX_Y, ERROR_BOX_W, ERROR_BOX_H, 0, 0, 0);
        errorBoxVisivel = 0;
    }
    if (c == '\n') {                                          // Enter premido: avanca 20px (nova linha) e repoe ">"
        int resultado = compararStrings();
        linhaAtual[linhaPos] = 0;
        linhaPos = 0;
        if (resultado != 2) {                                     //resultado 2 = "clear", que ja trata do ecra e do prompt
            termY += 20;
            if (chegouAoFimDaJanela()) {                            //nao ha mais espaco, "scroll": limpa o ecra e recomeca do topo
                clear();
            } else {
                termX = 10;
                desenharTexto(10, termY, "> ", 0, 255, 0, 2);           //para desenhar ">"
                termX = 10 + 2 * 8 * 2;                                 //Calcula a prox posicao ">"
            }
        }
    } else if (c == '\b') {
        if (linhaPos > 0) {
            linhaPos--;
            termX -= 8 * 2;
            desenharJanelas(termX, termY, 8 * 2, 8 * 2, 0, 0, 0);
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
            desenharChar(termX, termY, c, 0, 255, 0, 2);
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
    } else {
        errorMessage();
        return 0;
    }
}

void errorMessage() {
    desenharJanelas(ERROR_BOX_X, ERROR_BOX_Y, ERROR_BOX_W, ERROR_BOX_H, 0, 255, 0);                                          //moldura verde
    desenharJanelas(ERROR_BOX_X + border, ERROR_BOX_Y + border, ERROR_BOX_W - 2 * border, ERROR_BOX_H - 2 * border, 0, 0, 0); //interior preto

    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 20, "The command doesn't exist,", 0, 255, 0, 2);
    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 40, "please type 'help' for help", 0, 255, 0, 2);

    errorBoxVisivel = 1;                                //fica visivel ate o user comecar a escrever
}

void clear(){
    preencherEcra(0 , 0, 0);
    desenharTexto(10, 10, "LouOS Terminal", 0, 255, 0, 2);
    desenharTexto(10, 40, "> ", 0, 255, 0, 2);
    termX = 10 + 2 * 8 * 2;
    termY = 40;
}

void version() {
    desenharJanelas(ERROR_BOX_X, ERROR_BOX_Y, ERROR_BOX_W, ERROR_BOX_H, 0, 255, 0);                                          //moldura verde
    desenharJanelas(ERROR_BOX_X + border, ERROR_BOX_Y + border, ERROR_BOX_W - 2 * border, ERROR_BOX_H - 2 * border, 0, 0, 0); //interior preto

    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 20, "LouOS", 0, 255, 0, 2);
    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 40, "version 1.0", 0, 255, 0, 2);

    errorBoxVisivel = 1;
}

void uptime() {
    char buffer[10];
    intParaString(contadorTicks / 100, buffer);
    desenharJanelas(ERROR_BOX_X, ERROR_BOX_Y, ERROR_BOX_W, ERROR_BOX_H, 0, 255, 0);                                          //moldura verde
    desenharJanelas(ERROR_BOX_X + border, ERROR_BOX_Y + border, ERROR_BOX_W - 2 * border, ERROR_BOX_H - 2 * border, 0, 0, 0); //interior preto

    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 20, "LouOS", 0, 255, 0, 2);
    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 40, buffer, 0, 255, 0, 2);
    
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

    desenharJanelas(ERROR_BOX_X, ERROR_BOX_Y, ERROR_BOX_W, ERROR_BOX_H, 0, 255, 0);                                          //moldura verde
    desenharJanelas(ERROR_BOX_X + border, ERROR_BOX_Y + border, ERROR_BOX_W - 2 * border, ERROR_BOX_H - 2 * border, 0, 0, 0); //interior preto

    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 20, "LouOS - MB", 0, 255, 0, 2);
    desenharTexto(ERROR_BOX_X + border + 24, ERROR_BOX_Y + border + 40, buffer, 0, 255, 0, 2);
    
    errorBoxVisivel = 1;
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