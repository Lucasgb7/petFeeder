/*
 * File:   eeprom.c
 * Author: Jonath
 *
 * Created on July 4, 2020, 2:49 PM
 */


#include <xc.h>
#include "eeprom.h"

// Funcao para ler da memoria EEPROM
unsigned char readEEPROM(unsigned char address){
    EEADR = address; // Endereco para ser lido
    EECON1bits.EEPGD = 0; // Seleciona a Memoria de Dados EEPROM
    EECON1bits.RD = 1; // Inicializa o ciclo de leitura
    return EEDATA; // Retorna o dado 
}

// Funcao para escrever na memoria EEPROM
void writeEEPROM(unsigned char address, unsigned char data){
    unsigned char INTCON_SAVE; // Salvar o valor do registrador INTCON
    EEADR = address; // Endereco para escrever
    EEDATA = data; // Dado para escrever
    EECON1bits.EEPGD = 0; // Seleciona a Memoria de Dados EEPROM
    EECON1bits.WREN = 1; // Habilita escrita na EEPROM
    INTCON_SAVE=INTCON; // Backup regsitrador INCON 
    INTCON=0; // Desabilita interrupcao
    EECON2=0x55; // Sequencia para escrever na EEPROM
    EECON2=0xAA; // Sequencia para escrever na EEPROM
    EECON1bits.WR = 1; // Inicializa o ciclo de escrita
    INTCON = INTCON_SAVE; // Habilita a escrita
    EECON1bits.WREN = 0; // Desabilita a escrita
    while(PIR2bits.EEIF == 0); // Checa por outra operacao de escrita 
    PIR2bits.EEIF = 0; // Limpa o bit EEIF
}

void armazenaAlarmeEEPROM(char tabelaAlarmes[][4], int quantidadeAlarmes){
    for(unsigned int i=0; i < quantidadeAlarmes; i++){
        //for(unsigned int j=0; j < 4; j++) writeEEPROM((i*4)+1+j, tabelaAlarmes[i][j]);
    }
}

void carregaAlarmeEEPROM(char tabelaAlarmes[][4], int quantidadeAlarmes){
    for(unsigned int i=0; i < quantidadeAlarmes; i++){
        for(unsigned int j=0; j < 4; j++){
            //tabelaAlarmes[i][j] = readEEPROM((i*4)+1+j);
        }
    }
}