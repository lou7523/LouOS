#include "terminal.h"

int termX = 10;                 //Coordenadas do cursor
int termY = 60;                 //Coordenadas do cursor
char linhaAtual[256];
int linhaPos = 0;

void iniciarTerminal() {
    preencherEcra(0, 0, 0);                                     //Preenche o ecra a preto, "limpa o ecra"
    desenharTexto(10, 10, "LouOS Terminal", 0, 255, 0, 2);      //Escreve LouOS no topo da janela do terminal
    desenharTexto(10, 40, "> ", 0, 255, 0, 2);                  //Escreve '> ' para o user escrever em frente
    termX = 10 + 2 * 8 * 2;                                     //Coluna de inicio
    termY = 40;                                                 //Linha de Inicio
}

void terminalHandleChar(char c) {                               
    if (c == '\n') {                                            // Enter premido: avanca 20px (nova linha) e repoe ">"
        if (!compararStrings()) {
            linhaAtual[linhaPos] = 0;
            linhaPos = 0;
            termY += 20;                                        
            termX = 10;                                             
            desenharTexto(10, termY, "> ", 0, 255, 0, 2);           //para desenhar ">"
            termX = 10 + 2 * 8 * 2;                                 //Calcula a prox posicao ">"
        } else {
            errorMessage();
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
                termX = 10;                                         // e os pixeis a mais "sangravam" para a linha seguinte do framebuffer)
            }
            linhaAtual[linhaPos] = c;
            linhaPos++;
            desenharChar(termX, termY, c, 0, 255, 0, 2);
            termX += 8 * 2;
        }
    }
}

int compararStrings() {
    int igual = 1;
    for (int i = 0; i < linhaPos; i++) {
        if(linhaAtual[i] == "clear"[i]) {

        } else {
            igual = 0;
        }
    }
    if (igual) {
        clear();
        return 1;
    }
    return 0;
}



void clear(){
    preencherEcra(0 , 0, 0);
    desenharTexto(10, 10, "LouOS Terminal", 0, 255, 0, 2);
    desenharTexto(10, 40, "> ", 0, 255, 0, 2);
    termX = 10 + 2 * 8 * 2;
    termY = 40;
}