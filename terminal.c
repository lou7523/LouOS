#include "terminal.h"

int termX = 10;                 //Coordenadas do cursor
int termY = 60;                 //Coordenadas do cursor

void iniciarTerminal() {
    preencherEcra(0, 0, 0);                                     //Preenche o ecra a preto, "limpa o ecra"
    desenharTexto(10, 10, "LouOS Terminal", 0, 255, 0, 2);      //Escreve LouOS no topo da janela do terminal
    desenharTexto(10, 40, "> ", 0, 255, 0, 2);                  //Escreve '> ' para o user escrever em frente
    termX = 10 + 2 * 8 * 2;                                     //Coluna de inicio
    termY = 40;                                                 //Linha de Inicio
}

void terminalHandleChar(char c) {                               
    if (c == '\n') {                                            // Enter premido: avanca 20px (nova linha) e repoe ">"
        termY += 20;                                        
        termX = 10;                                             
        desenharTexto(10, termY, "> ", 0, 255, 0, 2);           //para desenhar ">"
        termX = 10 + 2 * 8 * 2;                                 //Calcula a prox posicao ">"
    } else if (c == '\b') {
        //Ainda nao fiz a funcao para backspace
    } else {
        desenharChar(termX, termY, c, 0, 255, 0, 2);            
        termX += 8 * 2;
    }
}