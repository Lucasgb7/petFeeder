/*
 * Título:      Alimentador automático (Pet Feeder)
 * File:        main.c
 * Author:      Jonath W. Herdt e Lucas J. Cunha
 * Disciplina:  Microcontroladores
 * Professor:   Paulo Roberto Oliveira Valim
 * Instituição: Universidade do Vale do Itajaí - UNIVALI
 * Created on June 4, 2020, 2:02 PM
 */


/* Algumas anotações:
 * 
 * Posicoes de memoria: 
 * 0 -> existem configuracoes
 * 1 à 12 -> alarmes
 * 13 -> quantidade de alarmes
 * 14 -> quantidade comida
 * 15 -> configuracao de som
 * 
 * Enderecos do PCF
 *  ENDL -> endereco
 *  3   =   Segundos (bit_7 = 1/ bit_6_to_4 = dezena / bit_3_to_0 = unidade)
 *  4   =   Minutos
 *  5   =   Horas
 *  6   =   Dias
 *  7   =   Dias da semana
 *  8   =   Meses
 *  9   =   Ano    
 *  10  =   Alarme minuto
 *  11  =   Alarme hora
 */

#include <xc.h>
#include <pic18f4520.h>
#include "I2C_Master.h"
#include "LCD.h"
#include "eeprom.h"

#pragma config MCLRE = ON, WDT = OFF, OSC = HS
#define _XTAL_FREQ 16000000

// Botoes
#define BTN_UP      PORTBbits.RB1
#define BTN_DOWN    PORTBbits.RB2 
#define BTN_OK      PORTBbits.RB3 
#define BTN_RETURN  PORTBbits.RB4 

// Configuracao inicial do microcontrolador
void setup(){
    TRISD = 0x00;   // portas de saida
    TRISB = 0xFF;   // portas de entrada
    TRISC = 0b11111000;   // portas de saida
    ADCON1 = 0x0F; // portas digitais
    
    // i2c
    SSPCON1 = 0b00101000;				// configura dispositivo como mestre
										// habilita porta serial e ativa pinos SDA E SCL <5>
										// ativa modo mestre <3:0>
	SSPADD = 3;						    // bit rate de 100kbps a Fosc = 16MHz ---- SSPADD = [(16M / 1M) / 4] -1
	SSPSTAT = 0b10000000;				// controle de inclinação desativado <7>
										// níveis de entrada conforme especificação I2C <6>
    
    DADO = 0b00000010;  // Ativa a interrupcao por alarme
    ENDL = 0;           // Register Control_1
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);
    
    DADO = 0b10000000;  // Ativa a interrupcao por alarme
    ENDL = 15;           // Register Control_1
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);
}

/*
 * Funcoes auxiliares de 
 * manipulacao de dados
*/

// converte inteiro para char
char* itoa(int value, char* result, int base_numerica) {
    // check that the base if valid
    if (base_numerica < 2 || base_numerica > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base_numerica;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base_numerica)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

// converte char em hexadecimal
unsigned char converteHex(char numero[]){
    unsigned char x;
    if(numero[0] == '0'){
        if(numero[1] == '0'){
            x = 0x00;
        }else if(numero[1] == '1'){
            x = 0x01;
        }else if(numero[1] == '2'){
            x = 0x02;
        }else if(numero[1] == '3'){
            x = 0x03;
        }else if(numero[1] == '4'){
            x = 0x04;
        }else if(numero[1] == '5'){
            x = 0x05;
        }else if(numero[1] == '6'){
            x = 0x06;
        }else if(numero[1] == '7'){
            x = 0x07;
        }else if(numero[1] == '8'){
            x = 0x08;
        }else{
            x = 0x09;
        }
    }else if(numero[0] == '1'){
        if(numero[1] == '0'){
            x = 0x10;
        }else if(numero[1] == '1'){
            x = 0x11;
        }else if(numero[1] == '2'){
            x = 0x12;
        }else if(numero[1] == '3'){
            x = 0x13;
        }else if(numero[1] == '4'){
            x = 0x14;
        }else if(numero[1] == '5'){
            x = 0x15;
        }else if(numero[1] == '6'){
            x = 0x16;
        }else if(numero[1] == '7'){
            x = 0x17;
        }else if(numero[1] == '8'){
            x = 0x18;
        }else{
            x = 0x19;
        }
    }else if(numero[0] == '2'){
        if(numero[1] == '0'){
            x = 0x20;
        }else if(numero[1] == '1'){
            x = 0x21;
        }else if(numero[1] == '2'){
            x = 0x22;
        }else if(numero[1] == '3'){
            x = 0x23;
        }else if(numero[1] == '4'){
            x = 0x24;
        }else if(numero[1] == '5'){
            x = 0x25;
        }else if(numero[1] == '6'){
            x = 0x26;
        }else if(numero[1] == '7'){
            x = 0x27;
        }else if(numero[1] == '8'){
            x = 0x28;
        }else{
            x = 0x29;
        }
    }else if(numero[0] == '3'){
        if(numero[1] == '0'){
            x = 0x30;
        }else if(numero[1] == '1'){
            x = 0x31;
        }else if(numero[1] == '2'){
            x = 0x32;
        }else if(numero[1] == '3'){
            x = 0x33;
        }else if(numero[1] == '4'){
            x = 0x34;
        }else if(numero[1] == '5'){
            x = 0x35;
        }else if(numero[1] == '6'){
            x = 0x36;
        }else if(numero[1] == '7'){
            x = 0x37;
        }else if(numero[1] == '8'){
            x = 0x38;
        }else{
            x = 0x39;
        }
    }else if(numero[0] == '4'){
        if(numero[1] == '0'){
            x = 0x40;
        }else if(numero[1] == '1'){
            x = 0x41;
        }else if(numero[1] == '2'){
            x = 0x42;
        }else if(numero[1] == '3'){
            x = 0x43;
        }else if(numero[1] == '4'){
            x = 0x44;
        }else if(numero[1] == '5'){
            x = 0x45;
        }else if(numero[1] == '6'){
            x = 0x46;
        }else if(numero[1] == '7'){
            x = 0x47;
        }else if(numero[1] == '8'){
            x = 0x48;
        }else{
            x = 0x49;
        }
    }else{
        if(numero[1] == '0'){
            x = 0x50;
        }else if(numero[1] == '1'){
            x = 0x51;
        }else if(numero[1] == '2'){
            x = 0x52;
        }else if(numero[1] == '3'){
            x = 0x53;
        }else if(numero[1] == '4'){
            x = 0x54;
        }else if(numero[1] == '5'){
            x = 0x55;
        }else if(numero[1] == '6'){
            x = 0x56;
        }else if(numero[1] == '7'){
            x = 0x57;
        }else if(numero[1] == '8'){
            x = 0x58;
        }else{
            x = 0x59;
        }
    }
    return x;
}

// Funcao para ordenar a matriz
void ordenaMatriz(char matriz[][4], int quantidade){
    for(int i=0; i < quantidade; i++){
        for(int j=i; j < quantidade; j++){
            char aux;
            if(matriz[j][0] < matriz[i][0]){
                aux = matriz[j][0];
                matriz[j][0] = matriz[i][0];
                matriz[i][0] = aux;
                aux = matriz[j][1];
                matriz[j][1] = matriz[i][1];
                matriz[i][1] = aux;
                aux = matriz[j][2];
                matriz[j][2] = matriz[i][2];
                matriz[i][2] = aux;
                aux = matriz[j][3];
                matriz[j][3] = matriz[i][3];
                matriz[i][3] = aux;
            }else if(matriz[j][1] < matriz[i][1]){
                matriz[j][0] = matriz[i][0];
                matriz[i][0] = aux;
                aux = matriz[j][1];
                matriz[j][1] = matriz[i][1];
                matriz[i][1] = aux;
                aux = matriz[j][2];
                matriz[j][2] = matriz[i][2];
                matriz[i][2] = aux;
                aux = matriz[j][3];
                matriz[j][3] = matriz[i][3];
                matriz[i][3] = aux;
            }else if(matriz[j][2] < matriz[i][2]){
                matriz[j][0] = matriz[i][0];
                matriz[i][0] = aux;
                aux = matriz[j][1];
                matriz[j][1] = matriz[i][1];
                matriz[i][1] = aux;
                aux = matriz[j][2];
                matriz[j][2] = matriz[i][2];
                matriz[i][2] = aux;
                aux = matriz[j][3];
                matriz[j][3] = matriz[i][3];
                matriz[i][3] = aux;
            }else if(matriz[j][3] < matriz[i][3]){
                matriz[j][0] = matriz[i][0];
                matriz[i][0] = aux;
                aux = matriz[j][1];
                matriz[j][1] = matriz[i][1];
                matriz[i][1] = aux;
                aux = matriz[j][2];
                matriz[j][2] = matriz[i][2];
                matriz[i][2] = aux;
                aux = matriz[j][3];
                matriz[j][3] = matriz[i][3];
                matriz[i][3] = aux;
            }
        }
    }
}


/* 
 *  Funcoes de interface
 */

// Funcoes de configuracao do petfeeder
void configuraAlarme(char horaAlarme[], char minutoAlarme[]){
    int horaAlarme_I, minutoAlarme_I;
    horaAlarme_I = atoi(horaAlarme); // converte string para inteiro
    minutoAlarme_I = atoi(minutoAlarme);
    
    gotoxy(0, 0); 
    writeString("Informe", 7);
    gotoxy(0, 1); 
    writeString("alarme:", 7);
    gotoxy(0, 2);
    writeString(horaAlarme, 2);
    gotoxy(2, 2);
    writeChar(':');
    gotoxy(3, 2);
    writeString(minutoAlarme, 2);
   
    int botaoUP = 0;
    int botaoDOWN = 0;
    
    horas:
    while(!BTN_OK){
        if(!BTN_UP) botaoUP = 0;
        if(BTN_UP && botaoUP == 0){
            botaoUP = 1;
            horaAlarme_I += 1;
            if(horaAlarme_I > 23) horaAlarme_I = 0;
            if(horaAlarme_I < 0) horaAlarme_I = 23;
            itoa(horaAlarme_I, horaAlarme, 10); // converte inteiro para string
            if(horaAlarme_I < 10){
                horaAlarme[1] = horaAlarme[0];
                horaAlarme[0] = '0';
            }
            gotoxy(0, 2);
            writeString(horaAlarme, 2);
            gotoxy(2, 2);
            writeChar(':');
        }
        if(!BTN_DOWN) botaoDOWN = 0;
        if(BTN_DOWN && botaoDOWN == 0){
            botaoDOWN = 1;
            horaAlarme_I -= 1;
            if(horaAlarme_I > 23) horaAlarme_I = 0;
            if(horaAlarme_I < 0) horaAlarme_I = 23;
            itoa(horaAlarme_I, horaAlarme, 10);
            if(horaAlarme_I < 10){
                horaAlarme[1] = horaAlarme[0];
                horaAlarme[0] = '0';
            }
            gotoxy(0, 2);
            writeString(horaAlarme, 2);
            gotoxy(2, 2);
            writeChar(':');
        }
    }
    
    __delay_ms(500);
    
    botaoUP = 0;
    botaoDOWN = 0;
    while(!BTN_OK){
        if(!BTN_UP) botaoUP = 0;
        if(BTN_UP && botaoUP == 0){
            botaoUP = 1;
            minutoAlarme_I += 1;
            if(minutoAlarme_I > 59) minutoAlarme_I = 0;
            if(minutoAlarme_I < 0) minutoAlarme_I = 59;
            itoa(minutoAlarme_I, minutoAlarme, 10); // converte inteiro para string
            if(minutoAlarme_I < 10){
                minutoAlarme[1] = minutoAlarme[0];
                minutoAlarme[0] = '0';    // ex: 09, 08, 07...
            }
            gotoxy(2, 2);
            writeChar(':');
            gotoxy(3, 2);
            writeString(minutoAlarme, 2);
        }
        if(!BTN_DOWN) botaoDOWN = 0;
        if(BTN_DOWN && botaoDOWN == 0){
            botaoDOWN = 1;
            minutoAlarme_I -= 1;
            if(minutoAlarme_I > 59) minutoAlarme_I = 0;
            if(minutoAlarme_I < 0) minutoAlarme_I = 59;
            itoa(minutoAlarme_I,minutoAlarme,10);
            if(minutoAlarme_I < 10){
                minutoAlarme[1] = minutoAlarme[0];
                minutoAlarme[0] = '0';
            }
            gotoxy(2, 2);
            writeChar(':');
            gotoxy(3, 2);
            writeString(minutoAlarme, 2);
        }
        if(BTN_RETURN){
            __delay_ms(500);
            goto horas;
        }
    }
    
    __delay_ms(500);
}

int printConfirmacao(){
    sendCMD(CLEAR);
    gotoxy(0, 0); 
    writeString("Adicionar outro", 15);
    gotoxy(0, 1); 
    writeString("horario?", 8);
    gotoxy(0, 2); 
    writeString("OK = SIM", 8);
    gotoxy(0, 3); 
    writeString("<< = NAO", 8);

    while(1){
        if(BTN_OK)
            return 1;   // configurar outro horario
        if(BTN_RETURN)
            return 0;   // proximo passo
    }
    
    __delay_ms(500);
}

int configuraQuantidade(){
    sendCMD(CLEAR);
    int quantidade = 1;
    char quantidade_c = quantidade + '0';
    
    gotoxy(0, 0); 
    writeString("Quantidade:", 11);
    gotoxy(0, 1);
    writeChar(quantidade_c);
    writeString("00 Gramas", 9);
    
    int botaoUP = 0;
    int botaoDOWN = 0;
    
    while(!BTN_OK){
        if(!BTN_UP) botaoUP = 0;
        if(BTN_UP && botaoUP == 0){
            botaoUP = 1;
            quantidade += 1;
            if(quantidade > 5) quantidade = 1;
            if(quantidade < 0) quantidade = 5;
            quantidade_c = quantidade + '0';
            gotoxy(0, 1);
            writeChar(quantidade_c);
            writeString("00 Gramas", 9);
        }
        if(!BTN_DOWN) botaoDOWN = 0;
        if(BTN_DOWN && botaoDOWN == 0){
            botaoDOWN = 1;
            quantidade -= 1;
            if(quantidade > 5) quantidade = 1;
            if(quantidade < 0) quantidade = 5;
            quantidade_c = quantidade + '0';
            gotoxy(0, 1);
            writeChar(quantidade_c);
            writeString("00 Gramas", 9);
        }
    }
    
    __delay_ms(500);
    
    return quantidade;
}

void configuraHorarioAtual(char hora[], char minuto[], char segundo[]){
    int hora_I, minuto_I, segundo_I;
    hora_I = atoi(hora); // converte string para inteiro
    minuto_I = atoi(minuto);
    segundo_I = atoi(segundo);
    
    gotoxy(0, 0); 
    writeString("Informe", 7);
    gotoxy(0, 1); 
    writeString("horario:", 8);
    gotoxy(0, 2);
    writeString(hora, 2);
    gotoxy(2, 2);
    writeChar(':');
    gotoxy(3, 2);
    writeString(minuto, 2);
    gotoxy(5, 2);
    writeChar(':');
    gotoxy(6, 2);
    writeString(segundo, 2);
   
    int botaoUP = 0;
    int botaoDOWN = 0;   
    horas:
    
    while(!BTN_OK){
        if(!BTN_UP) botaoUP = 0;
        if(BTN_UP && botaoUP == 0){
            botaoUP = 1;
            hora_I += 1;
            if(hora_I > 23) hora_I = 0;
            if(hora_I < 0) hora_I = 23;
            itoa(hora_I, hora, 10); // converte inteiro para string
            if(hora_I < 10){
                hora[1] = hora[0];
                hora[0] = '0';
            }
            gotoxy(0, 2);
            writeString(hora, 2);
            gotoxy(2, 2);
            writeChar(':');
            gotoxy(5, 2);
            writeChar(':');
        }
        if(!BTN_DOWN) botaoDOWN = 0;
        if(BTN_DOWN && botaoDOWN == 0){
            botaoDOWN = 1;
            hora_I -= 1;
            if(hora_I > 23) hora_I = 0;
            if(hora_I < 0) hora_I = 23;
            itoa(hora_I, hora, 10);
            if(hora_I < 10){
                hora[1] = hora[0];
                hora[0] = '0';
            }
            gotoxy(0, 2);
            writeString(hora, 2);
            gotoxy(2, 2);
            writeChar(':');
            gotoxy(5, 2);
            writeChar(':');
        }
    }    
    __delay_ms(500);
    minutos:
    botaoUP = 0;
    botaoDOWN = 0;   
    while(!BTN_OK){
        if(!BTN_UP) botaoUP = 0;
        if(BTN_UP && botaoUP == 0){
            botaoUP = 1;
            minuto_I += 1;
            if(minuto_I > 59) minuto_I = 0;
            if(minuto_I < 0) minuto_I = 59;
            itoa(minuto_I, minuto, 10); // converte inteiro para string
            if(minuto_I < 10){
                minuto[1] = minuto[0];
                minuto[0] = '0';    // ex: 09, 08, 07...
            }
            gotoxy(2, 2);
            writeChar(':');
            gotoxy(3, 2);
            writeString(minuto, 2);
            gotoxy(5, 2);
            writeChar(':');
        }
        if(!BTN_DOWN) botaoDOWN = 0;
        if(BTN_DOWN && botaoDOWN == 0){
            botaoDOWN = 1;
            minuto_I -= 1;
            if(minuto_I > 59) minuto_I = 0;
            if(minuto_I < 0) minuto_I = 59;
            itoa(minuto_I,minuto,10);
            if(minuto_I < 10){
                minuto[1] = minuto[0];
                minuto[0] = '0';
            }
            gotoxy(2, 2);
            writeChar(':');
            gotoxy(3, 2);
            writeString(minuto, 2);
            gotoxy(5, 2);
            writeChar(':');
        }
        if(BTN_RETURN){
            __delay_ms(500);
            goto horas;
        }
    }
     __delay_ms(500);
     
    botaoUP = 0;
    botaoDOWN = 0;  
     
    //segundos
    while(!BTN_OK){
        if(!BTN_UP) botaoUP = 0;
        if(BTN_UP && botaoUP == 0){
            botaoUP = 1;
            segundo_I += 1;
            if(segundo_I > 59) segundo_I = 0;
            if(segundo_I < 0) segundo_I = 59;
            itoa(segundo_I, segundo, 10); // converte inteiro para string
            if(segundo_I < 10){
                segundo[1] = segundo[0];
                segundo[0] = '0';    // ex: 09, 08, 07...
            }
            gotoxy(5, 2);
            writeChar(':');
            gotoxy(6, 2);
            writeString(segundo, 2);
        }
        if(!BTN_DOWN) botaoDOWN = 0;
        if(BTN_DOWN && botaoDOWN == 0){
            botaoDOWN = 1;
            segundo_I -= 1;
            if(segundo_I > 59) segundo_I = 0;
            if(segundo_I < 0) segundo_I = 59;
            itoa(segundo_I,segundo,10);
            if(segundo_I < 10){
                segundo[1] = segundo[0];
                segundo[0] = '0';
            }
            gotoxy(5, 2);
            writeChar(':');
            gotoxy(6, 2);
            writeString(segundo, 2);
        }
        if(BTN_RETURN){
            __delay_ms(500);
            goto minutos;
        }
    }
    __delay_ms(500);
}

int configuraSom(){
    sendCMD(CLEAR);
    gotoxy(0, 0); 
    writeString("Acionar som", 11);
    gotoxy(0, 1); 
    writeString("ao despejar?", 12);
    gotoxy(0, 2); 
    writeString("OK = SIM", 8);
    gotoxy(0, 3); 
    writeString("<< = NAO", 8);

    while(1){
        if(BTN_OK)
            return 1;   // configurar outro horario
        if(BTN_RETURN)
            return 0;   // proximo passo
    }
    
    __delay_ms(500);
}

void exibeInformacoes(char horaAtual[], char proxHora[], char proxMinuto[], char ultHora[], char ultMinuto[], char quantidade){
    gotoxy(0, 0); 
    writeString("Horario: ", 10); 
    writeString(horaAtual, 5);
    gotoxy(0, 1);
    writeString("proxima: ", 10);
    writeString(proxHora, 2);
    writeChar(':');
    writeString(proxMinuto, 2);
    gotoxy(0, 2);
    writeString("anterior: ", 10);
    writeString(ultHora, 2);
    writeChar(':');
    writeString(ultMinuto, 2);
    gotoxy(0, 3);
    writeString("Quant:", 6);
    writeChar(quantidade);
    writeString("00 Gramas", 9);
}

void exibeDespejando(int quantidade){
    char quantidadeC = quantidade + '0';
    sendCMD(CLEAR);
    gotoxy(0, 0); 
    writeString("Despejando", 10);
    gotoxy(0, 1); 
    writeChar(quantidadeC);
    writeString("00 Gramas", 9);
    gotoxy(0, 2); 
    writeString("de racao", 8);
}

int exibeMenu(){
    sendCMD(CLEAR);
    gotoxy(0, 0); 
    writeString("Configurar:", 11);
    gotoxy(0, 1); 
    writeString("Alarmes", 7);
    gotoxy(0, 3); 
    writeString("< (+)      (-) >", 16);
    
    int opcao = 1;
    int altera = 0;
    while(1){
        if(altera == 1){
            altera = 0;
            sendCMD(CLEAR);
            gotoxy(0, 0); 
            writeString("Configurar:", 11);
            gotoxy(0, 1);
            if(opcao == 1)
                writeString("Alarmes", 7);
            else if(opcao == 2)
                writeString("Quantidade", 10);
            else if(opcao == 3)
                writeString("Som", 3);
            else
                writeString("Horario", 7);
            gotoxy(0, 3); 
            writeString("< (+)      (-) >", 16);
        }
        if(BTN_UP){
            opcao += 1;
            if(opcao < 1) opcao = 4;
            if(opcao > 4) opcao = 1;
            altera = 1;
        }
        if(BTN_DOWN){
            opcao -= 1;
            if(opcao < 1) opcao = 4;
            if(opcao > 4) opcao = 1;
            altera = 1;
        }
        if(BTN_OK){
            __delay_ms(500);
            return opcao;
        }
        if(BTN_RETURN){
            return 5;
        }
        __delay_ms(10);
    }
}


/*
 * Funcionalidade do PetFeeder
 * 
 */
void setAlarme(char hora[], char minuto[]){
    unsigned char horaChar = converteHex(hora);
    ENDL = 11; //hora alarme
    ESCRITA_PCF8523T(ENDH, ENDL, horaChar);
    
    unsigned char minutoChar = converteHex(minuto);
    ENDL = 10; //minuto alarme
    ESCRITA_PCF8523T(ENDH, ENDL, minutoChar);
}

int encontraAlarmeAtual(char tabelaAlarmes[][4], char horaAtual[], char minutoAtual[], unsigned int quantidade){
    int i = 0;
    while(i < quantidade){
        if(tabelaAlarmes[i][0] < horaAtual[0]){
            i++;
        }else if(tabelaAlarmes[i][1] < horaAtual[1]){
            i++;
        }else if(tabelaAlarmes[i][2] < minutoAtual[0]){
            i++;
        }else if(tabelaAlarmes[i][3] < minutoAtual[1]){
            i++;
        }else{
            return i;
        }
    } 
    return i;
}

void despejarRacao(int som, int quantidade){
    //ativa o motor
    CCPR1L = 199;
            
    //liga o led
    PORTCbits.RC0 = 1;
            
    //liga buzzer caso configurado
    if(som) PORTDbits.RD3 = 1;
            
    exibeDespejando(quantidade);
            
    for(int i=0; i < quantidade; i++){
        __delay_ms(1000);
        CCPR1L = 199;
    }
}

void paraDespejar(){
    //desliga o motor
    CCPR1L = 0;
            
    //desliga o led
    PORTCbits.RC0 = 0;
            
    //desliga som
    PORTDbits.RD3 = 0;
}

void main(void) {
    setup(); // configuracoes iniciais
    start(); // inicía o LCD
    pLCD = &PORTD;  // LCD le a porta D diretamente para configuracoes iniciais
    
    gotoxy(0, 0); 
    writeString("Iniciando", 9);
    gotoxy(0, 1);
    writeString("dispositivo...", 14);
     __delay_ms(1000);   
    sendCMD(CLEAR);
    
    unsigned char horaAtual[2] = "00";
    unsigned char minutoAtual[2] = "00";
    unsigned char segundoAtual[2] = "00";
    char horaAlarme[2], minutoAlarme[2], tabelaAlarmes[3][4], horaAlarmeDispositivo[2], minutoAlarmeDispositivo[2], antAlarmeHora[2], antAlarmeMinuto[2];
    unsigned int quantidadeAlarmes, alarmeAtual, alarmeAnterior, quantidade, som, sair;
    
    if(readEEPROM(0x00) == '1'){ // caso tenha dados na memoria, carrega a mesma
        quantidadeAlarmes = atoi(readEEPROM(0x0D));
        carregaAlarmeEEPROM(tabelaAlarmes, quantidadeAlarmes);
        quantidade = atoi(readEEPROM(0xE));
        som = atoi(readEEPROM(0xF));
    }else{ // realiza a configuracao
        configuraHorarioAtual(horaAtual, minutoAtual, segundoAtual);
    
        ENDL= 3; //segundos
        ESCRITA_PCF8523T(ENDH, ENDL, converteHex(segundoAtual)); // escreve no módulo RTC
        __delay_ms(10);

        ENDL= 4; //minutos
        ESCRITA_PCF8523T(ENDH, ENDL, converteHex(minutoAtual));
        __delay_ms(10);

        ENDL= 5; //horas
        ESCRITA_PCF8523T(ENDH, ENDL, converteHex(horaAtual));
        __delay_ms(10);
    
        quantidadeAlarmes = 0;
        sair = 0;
        do{
            horaAlarme[0] = '0';
            horaAlarme[1] = '0';
            minutoAlarme[0] = '0';
            minutoAlarme[1] = '0';
            sendCMD(CLEAR);
            configuraAlarme(horaAlarme, minutoAlarme);
            tabelaAlarmes[quantidadeAlarmes][0] = horaAlarme[0];
            tabelaAlarmes[quantidadeAlarmes][1] = horaAlarme[1];

            tabelaAlarmes[quantidadeAlarmes][2] = minutoAlarme[0];
            tabelaAlarmes[quantidadeAlarmes][3] = minutoAlarme[1];

            quantidadeAlarmes += 1;
            if(quantidadeAlarmes > 2) sair = 1;
            else if(printConfirmacao() == 0) sair = 1;
        }while(sair == 0);

        ordenaMatriz(tabelaAlarmes, quantidadeAlarmes);

        armazenaAlarmeEEPROM(tabelaAlarmes, quantidadeAlarmes);
        writeEEPROM(0x0D, quantidadeAlarmes);
    
        quantidade = configuraQuantidade();
        writeEEPROM(0x0E, quantidade);  //  escreve na memoria EEPROM (1 byte)

        som = configuraSom();
        writeEEPROM(0x0F, som);     // escreve na memoria EEPROM (1 byte)

        writeEEPROM(0x00, '1');   // escreve na memoria EEPROM que existem configuracoes salvas
    }
    
    if(quantidadeAlarmes == 1){
        alarmeAtual = 0;
    }else{
        alarmeAtual = encontraAlarmeAtual(tabelaAlarmes, horaAtual, minutoAtual, quantidadeAlarmes);
    }
    
    horaAlarmeDispositivo[0] = tabelaAlarmes[alarmeAtual][0];
    horaAlarmeDispositivo[1] = tabelaAlarmes[alarmeAtual][1];

    minutoAlarmeDispositivo[0] = tabelaAlarmes[alarmeAtual][2];
    minutoAlarmeDispositivo[1] = tabelaAlarmes[alarmeAtual][3];

    if(quantidadeAlarmes == 1){
        alarmeAnterior = 0;
    }else{
        alarmeAnterior = alarmeAtual - 1;
        if(alarmeAnterior < 0) alarmeAnterior = quantidadeAlarmes-1;
    }
    
    antAlarmeHora[0] = tabelaAlarmes[alarmeAnterior][0];
    antAlarmeHora[1] = tabelaAlarmes[alarmeAnterior][1];

    antAlarmeMinuto[0] = tabelaAlarmes[alarmeAnterior][2];
    antAlarmeMinuto[1] = tabelaAlarmes[alarmeAnterior][3];

    setAlarme(horaAlarmeDispositivo, minutoAlarmeDispositivo);
        
    PR2 = 199;                //periodo de 5000Hz para um oscilador de 16MHz
    CCPR1L = 0;               //Duty Cycle de 50%
    CCP1CONbits.DC1B = 0;     //dois bits menos significativos como zero
    CCP1CONbits.CCP1M = 0x0C; //configura modulo CCP para operar como PWM (00001100)
    T2CON = 0x05;             //Prescaler TMR2 como 1:4 ativando timer 2
    
    sendCMD(CLEAR);
    
    char dadoHoraAtual[6];
    leituraHora(dadoHoraAtual);
    exibeInformacoes(dadoHoraAtual, horaAlarmeDispositivo, minutoAlarmeDispositivo, antAlarmeHora, antAlarmeMinuto, quantidade + '0');
    
    int i = 0;
    
    while(1){
        if(i == 500){
            leituraHora(dadoHoraAtual);
            exibeInformacoes(dadoHoraAtual, horaAlarmeDispositivo, minutoAlarmeDispositivo, antAlarmeHora, antAlarmeMinuto, quantidade + '0');
            i=0;
        }
        if(PORTBbits.RB0){
            DADO = 0b00001000; 
            ENDL = 1; // Register de Controle 2
            ESCRITA_PCF8523T(ENDH, ENDL, DADO); // escreve no módulo RTC
            
            despejarRacao(som, quantidade);
            paraDespejar();
            
            alarmeAtual += 1;
            if(alarmeAtual >= quantidadeAlarmes)
                alarmeAtual = 0;

            horaAlarmeDispositivo[0] = tabelaAlarmes[alarmeAtual][0];
            horaAlarmeDispositivo[1] = tabelaAlarmes[alarmeAtual][1];
            
            minutoAlarmeDispositivo[0] = tabelaAlarmes[alarmeAtual][2];
            minutoAlarmeDispositivo[1] = tabelaAlarmes[alarmeAtual][3];

            setAlarme(horaAlarmeDispositivo, minutoAlarmeDispositivo);
            
            alarmeAnterior += 1;
            if(alarmeAnterior >= quantidadeAlarmes) alarmeAnterior = 0;
            
            antAlarmeHora[0] = tabelaAlarmes[alarmeAnterior][0];
            antAlarmeHora[1] = tabelaAlarmes[alarmeAnterior][1];
            
            antAlarmeMinuto[0] = tabelaAlarmes[alarmeAnterior][2];
            antAlarmeMinuto[1] = tabelaAlarmes[alarmeAnterior][3];
        }
        if(BTN_OK){
            despejarRacao(som, quantidade);
            paraDespejar();
        }
        if(BTN_RETURN){
            int opcao = exibeMenu();
            if(opcao == 1){ //configura alarmes
                quantidadeAlarmes = 0;
                sair = 0;
                do{
                    horaAlarme[0] = '0';
                    horaAlarme[1] = '0';
                    minutoAlarme[0] = '0';
                    minutoAlarme[1] = '0';
                    sendCMD(CLEAR);
                    configuraAlarme(horaAlarme, minutoAlarme);
                    tabelaAlarmes[quantidadeAlarmes][0] = horaAlarme[0];
                    tabelaAlarmes[quantidadeAlarmes][1] = horaAlarme[1];
                    tabelaAlarmes[quantidadeAlarmes][2] = minutoAlarme[0];
                    tabelaAlarmes[quantidadeAlarmes][3] = minutoAlarme[1];
                    quantidadeAlarmes += 1;

                    if(quantidadeAlarmes > 2) sair = 1;
                    else if(printConfirmacao() == 0) sair = 1;
                }while(sair == 0);
                ordenaMatriz(tabelaAlarmes, quantidadeAlarmes);
                
                alarmeAtual = encontraAlarmeAtual(tabelaAlarmes, horaAtual, minutoAtual, quantidadeAlarmes);
    
                horaAlarmeDispositivo[0] = tabelaAlarmes[alarmeAtual][0];
                horaAlarmeDispositivo[1] = tabelaAlarmes[alarmeAtual][1];

                minutoAlarmeDispositivo[0] = tabelaAlarmes[alarmeAtual][2];
                minutoAlarmeDispositivo[1] = tabelaAlarmes[alarmeAtual][3];

                alarmeAnterior = alarmeAtual - 1;
                if(alarmeAnterior < 0) alarmeAnterior = quantidadeAlarmes-1;

                antAlarmeHora[0] = tabelaAlarmes[alarmeAnterior][0];
                antAlarmeHora[1] = tabelaAlarmes[alarmeAnterior][1];

                antAlarmeMinuto[0] = tabelaAlarmes[alarmeAnterior][2];
                antAlarmeMinuto[1] = tabelaAlarmes[alarmeAnterior][3];

                setAlarme(horaAlarmeDispositivo, minutoAlarmeDispositivo);
                
                armazenaAlarmeEEPROM(tabelaAlarmes, quantidadeAlarmes);
                writeEEPROM(0x0D, quantidadeAlarmes);
            }else if(opcao == 2){ //configura quantidade
                quantidade = configuraQuantidade();
                writeEEPROM(0x0E, quantidade);
            }else if(opcao == 3){ //configura som
                som = configuraSom();
                writeEEPROM(0x0F, quantidade);
            }else if(opcao == 4){ // configura horario atual
                sendCMD(CLEAR);
                configuraHorarioAtual(horaAtual, minutoAtual, segundoAtual);
                ENDL= 3; //segundos
                ESCRITA_PCF8523T(ENDH, ENDL, converteHex(segundoAtual));
                __delay_ms(10);
                ENDL= 4; //minutos
                ESCRITA_PCF8523T(ENDH, ENDL, converteHex(minutoAtual));
                __delay_ms(10);
                ENDL= 5; //horas
                ESCRITA_PCF8523T(ENDH, ENDL, converteHex(horaAtual));
                __delay_ms(10);
            }else{
                leituraHora(dadoHoraAtual);
                exibeInformacoes(dadoHoraAtual, horaAlarmeDispositivo, minutoAlarmeDispositivo, antAlarmeHora, antAlarmeMinuto, quantidade + '0');
            }
        }
        i += 1;
        __delay_ms(10);
    }
    return; // --------------- FIM DO PROGRAMA ---------------
}