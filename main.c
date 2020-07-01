/*
 * Título:      Alimentador automático (Pet Feeder)
 * File:        main.c
 * Author:      Jonath W. Herdt e Lucas J. Cunha
 * Disciplina:  Microcontroladores
 * Professor:   Paulo Roberto Oliveira Valim
 * Instituição: Universidade do Vale do Itajaí - UNIVALI
 * Created on June 4, 2020, 2:02 PM
 */
#include <xc.h>
#include "I2C_Master.h"

#pragma config MCLRE = ON, WDT = OFF, OSC = HS
#define _XTAL_FREQ 16000000

// Botoes
#define BTN_UP      PORTBbits.RB0
#define BTN_DOWN    PORTBbits.RB1 
#define BTN_OK      PORTBbits.RB2 
#define BTN_RETURN  PORTBbits.RB3 

// Define display
#define CLEAR   0x01

unsigned char ESCRITA_PCF8523T  (unsigned char ENDH, unsigned char ENDL, char DADO);
unsigned char LEITURA_PCF8523T (unsigned char _ENDH, unsigned char _ENDL);
void leituraHora(char *hour);

// PCF8523
char buf [17];
unsigned char ENDH = 0b11010000; // Endereco para o PCF8523T
unsigned char ENDL = 1;
char DADO;
unsigned char TEMP;

// Comandos do display (4 bits)
typedef struct tDisplayPort{
    char RS: 1;
    char R_W: 1;
    char E: 1;
    char lixo: 1;
    char NData: 4;
};
struct tDisplayPort * pLCD;

// Ativa e desativa o 'enable' para um pulso
void pulseEnable(){ 
    __delay_us(1000);
    pLCD->E = 1;
    __delay_us(1000);
    pLCD->E = 0;
}

// Tempo entre um pulso
void waitIdle(){
    char aux = 0xFF;
    TRISD = 0xF0;
    pLCD->RS = 0;
    pLCD->R_W = 1;
    while ( aux&0x80 ){
        pLCD->E = 1;
        __delay_us(1000);
        pLCD->E = 0;
        __delay_us(1000);
        aux = (pLCD->NData) << 4;
        pLCD->E = 1;
        __delay_us(1000);
        pLCD->E = 0;
        __delay_us(1000);
        aux = aux | (pLCD->NData);
    }
    pLCD->R_W = 0;
    TRISD = 0x00;
}

// Funcao para enviar o cursor
void sendCMD(char data){
    pLCD->RS = 0;
    pLCD->R_W = 0;
    pLCD->NData = data >> 4;    // Leftshift para deslocamento dos 8 bits
    pulseEnable();
    pLCD->NData = data&0x0F;
    pulseEnable();
    waitIdle();
}

// Escreve um dado no LCD
void writeChar(char data){
    pLCD->RS = 1;   // RS para escrita
    pLCD->R_W = 0;
    pLCD->NData = data >> 4;    // Deslocamento dos bits para utilizar todos
    pulseEnable();  
    pLCD->RS = 1;
    pLCD->NData = data&0x0F;
    pulseEnable();
    waitIdle();
}

// Escreve uma palavra no LCD
void writeString(char c[], int tam)                  // Function to print Strings on LCD
{
    for(int i = 0; i < tam; i++){
        writeChar(c[i]);
    }  
}

// envia o cursor para alguma posição do LCD
void gotoxy(char x, char y){
    if ((x < 15 || x > 0) && (y < 3 || y > 0)){ // Verifica se tá dentro do tamanho normal
        if(y == 0){
            sendCMD(0x80 + x);  // N = 0 endereço inicial: 0x80
        }else if(y == 1){
            sendCMD(0xC0 + x); // N = 1 endereço inicial: 0xC0
        }else if(y == 2){
            sendCMD(0x90 + x); // N = 2 endereço inicial: 0x90
        }else{
            sendCMD(0xD0 + x); // N = 3 endereço inicial: 0x90
        }
    } else {
        sendCMD(CLEAR);
    }
}

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
    
    DADO = 0b00000010; // Ativa a interrupcao por alarme
    ENDL = 0; // Register Control_1
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);
    DADO = 0b00001000; // Seta flag quando alarme é disparado, limpa flag quando não há interrupção
    ENDL = 1; // Register Control_2
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);
}
// Pulso direto na porta D
void enable(){
    __delay_ms(1);
    PORTD = PORTD|0x04; // Pulso do Enable = 1 (Valor + 0000 0100)
    __delay_ms(1);
    PORTD = PORTD&0xFB;  // Pulso do Enable = 0 (Valor + 1111 1011)
    __delay_ms(5);
}
// Configuracoes iniciais
void start(){
    PORTD = 0x20;       // Function set
    enable();
    PORTD = 0x20;       // Function set
    enable();
    PORTD = 0x80;       // Fonte
    enable();
    
    
    PORTD = 0x00;       // Display on/off
    enable();
    PORTD = 0xE0;
    enable();
    
    PORTD = 0x00;       // Entry mode set
    enable();
    PORTD = 0x60;
    enable();
}

//Funcoes de C que nao tem no xc.h
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

char converteHex(char numero[]){
    char x;
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
        }else if(BTN_DOWN){
            opcao -= 1;
            if(opcao < 1) opcao = 4;
            if(opcao > 4) opcao = 1;
            altera = 1;
        }else if(BTN_OK){
            __delay_ms(500);
            return opcao;
        }
        __delay_ms(10);
    }
}

void setAlarme(char hora[], char minuto[]){
    char horaChar = converteHex(hora);
    ENDL = 11; //hora alarme
    ESCRITA_PCF8523T(ENDH, ENDL, horaChar);
    
    char minutoChar = converteHex(minuto);
    ENDL = 10; //minuto alarme
    ESCRITA_PCF8523T(ENDH, ENDL, minutoChar);
}

//nao ta funcionando
void ordenaMatriz(char matriz[][4], int quantidade){
    for(int i=0; i < quantidade; i++){
        for(int j=i; j < quantidade; j++){
            char aux;
            if(matriz[j][0] < matriz[i][0]){
                aux = matriz[j][0];
                matriz[j][0] = matriz[i][0];
                matriz[i][0] = aux;
            }else if(matriz[j][1] < matriz[i][1]){
                aux = matriz[j][1];
                matriz[j][1] = matriz[i][1];
                matriz[i][1] = aux;
            }else if(matriz[j][2] < matriz[i][2]){
                aux = matriz[j][2];
                matriz[j][2] = matriz[i][2];
                matriz[i][2] = aux;
            }else if(matriz[j][3] < matriz[i][3]){
                aux = matriz[j][3];
                matriz[j][3] = matriz[i][3];
                matriz[i][3] = aux;
            }
        }
    }
}

int encontraAlarmeAtual(char tabelaAlarmes[][4], char horaAtual[], char minutoAtual[], int quantidade){
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
    
    char horaAtual[2] = "00";
    char minutoAtual[2] = "00";
    char segundoAtual[2] = "00";

    configuraHorarioAtual(horaAtual, minutoAtual, segundoAtual);
    char horaAtualChar = converteHex(horaAtual);
    char minutoAtualChar = converteHex(minutoAtual);
    char segundoAtualchar = converteHex(segundoAtual);
    // DADO = segundoAtual; // converter para hexadecimal
        // ENDL = 3; 
        // configura horario atual no PCF
                /*  ENDL    -   DEFINIR HORARIO ATUAL
         *  3   =   Segundos (bit_7 = 1/ bit_6_to_4 = dezena / bit_3_to_0 = unidade)
         *  4   =   Minutos
         *  5   =   Horas
         *  6   =   Dias
         *  7   =   Dias da semana
         *  8   =   Meses
         *  8   =   Ano    
         */
    ENDL= 3; //segundos
    ESCRITA_PCF8523T(ENDH, ENDL, segundoAtualchar);
	__delay_ms(10);
    
    ENDL= 4; //minutos
    ESCRITA_PCF8523T(ENDH, ENDL, minutoAtualChar);
    __delay_ms(10);
    
    ENDL= 5; //horas
    ESCRITA_PCF8523T(ENDH, ENDL, horaAtualChar);
	__delay_ms(10);
    
    char horaAlarme[2];
    char minutoAlarme[2];
    char tabelaAlarmes[3][4];
    int quantidadeAlarmes = 0;
    
    int sair = 0;
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
    
    int alarmeAtual = encontraAlarmeAtual(tabelaAlarmes, horaAtual, minutoAtual, quantidadeAlarmes);
    
    char horaAlarmeDispositivo[2];
    horaAlarmeDispositivo[0] = tabelaAlarmes[alarmeAtual][0];
    horaAlarmeDispositivo[1] = tabelaAlarmes[alarmeAtual][1];
    
    char minutoAlarmeDispositivo[2];
    minutoAlarmeDispositivo[0] = tabelaAlarmes[alarmeAtual][2];
    minutoAlarmeDispositivo[1] = tabelaAlarmes[alarmeAtual][3];
    
    int alarmeAnterior = alarmeAtual - 1;
    if(alarmeAnterior < 0) alarmeAnterior = quantidadeAlarmes-1;
    
    char antAlarmeHora[2];
    antAlarmeHora[0] = tabelaAlarmes[alarmeAnterior][0];
    antAlarmeHora[1] = tabelaAlarmes[alarmeAnterior][1];
    
    char antAlarmeMinuto[2];
    antAlarmeMinuto[0] = tabelaAlarmes[alarmeAnterior][2];
    antAlarmeMinuto[1] = tabelaAlarmes[alarmeAnterior][3];
    
    setAlarme(horaAlarmeDispositivo, minutoAlarmeDispositivo);
    
    int quantidade = configuraQuantidade();
    
    int som = configuraSom();
    
    PR2 = 199;                //periodo de 5000Hz para um oscilador de 16MHz
    CCPR1L = 0;               //Duty Cycle de 50%
    CCP1CONbits.DC1B = 0;     //dois bits menos significativos como zero
    CCP1CONbits.CCP1M = 0x0C; //configura modulo CCP para operar como PWM (00001100)
    T2CON = 0x05;             //Prescaler TMR2 como 1:4 ativando timer 2
    
    sendCMD(CLEAR);
    
    char dadoHoraAtual[6];
    leituraHora(dadoHoraAtual);
    
    int i = 0;
    
    while(1){
        if(i % 500 == 0){
            leituraHora(dadoHoraAtual);
            exibeInformacoes(dadoHoraAtual, horaAlarmeDispositivo, minutoAlarmeDispositivo, antAlarmeHora, antAlarmeMinuto, quantidade + '0');
        }
        if(PORTBbits.RB4){
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
            }else if(opcao == 2){ //configura quantidade
                quantidade = configuraQuantidade();
            }else if(opcao == 3){ //configura som
                som = configuraSom();
            }else{
                sendCMD(CLEAR);
                configuraHorarioAtual(horaAtual, minutoAtual, segundoAtual);
                horaAtualChar = converteHex(horaAtual);
                minutoAtualChar = converteHex(minutoAtual);
                segundoAtualchar = converteHex(segundoAtual);
                ENDL= 3; //segundos
                ESCRITA_PCF8523T(ENDH, ENDL, segundoAtualchar);
                __delay_ms(10);
                ENDL= 4; //minutos
                ESCRITA_PCF8523T(ENDH, ENDL, minutoAtualChar);
                __delay_ms(10);
                ENDL= 5; //horas
                ESCRITA_PCF8523T(ENDH, ENDL, horaAtualChar);
                __delay_ms(10);
            }
        }
        i++;
        __delay_ms(10);
    }      // Loop
    return;         // Fim do programa
        
    /*
    1º Ligar dispositivo
     * Ascender LED de indicação (ON/OFF) [Controlado no PROTEUS]
     * Mostrar no display mensagem de inicialização (Ex.: "Iniciando dispositivo...")

    2º Mensagens
     * Verificar se já tem configuração pre estabelecida
     * Informar horário de despejo
     * Informar quantidade a ser despejada
     * Configurar botoes de OK, Voltar, + e -
     * Confirmação de configuração (OK)
     
    3º Estudar função alarme do módulo RTC
     * Analisar a posibilidade de mais de um alarme
     
    4º Verificar a força e tempo do motor
    */    
}

// Funcao de escrita do PCF8523
unsigned char ESCRITA_PCF8523T (unsigned char ENDH, unsigned char ENDL, char DADO){
    char x;								//declaração de variável local 
	x = I2C_LIVRE ();					//chamada à função com retorno de valor. Verifica se barramento está livre
	if (x == 0)							//se barramento ocupado, aborta transmissão e retorna
	{
		I2C_STOP();						//gera bit STOP
		return -1;						//erro na transmissão
	}
	else 								//barramento livre
	{
		I2C_START();					//barramento livre, gera condição START
		//TEMP= ENDH << 1;				//rotaciona END_I2C para a esquerda (insere bit R_W para escrita)  - nao precisa, pq meu endereco ja ta certo
		I2C_TRANSMITE(ENDH);			//transmite endereço alto
		if (!I2C_TESTA_ACK())	 		//se erro na transmissão, aborta transmissão
		{
			I2C_STOP();					//gera bit STOP
			return -1;					//erro na transmissão
 		}
		I2C_TRANSMITE(ENDL);			//transmite endereço baixo
		if (!I2C_TESTA_ACK())			//se erro na transmissão, aborta transmissão
		{
			I2C_STOP();					//gera bit STOP
			return -1;					//erro na transmissão
		}
		I2C_TRANSMITE(DADO);			//transmite dado
		if (!I2C_TESTA_ACK())	 		//se erro na transmissão, aborta transmissão
		{
			I2C_STOP();					//gera bit STOP
			return -1;					//erro na transmissão
 		}
		I2C_STOP();						//gera bit STOP
        
        
		return DADO;					//transmissão feita com sucesso
	}
}

// Funcao de leitura do PCF8523T
unsigned char LEITURA_PCF8523T (unsigned char _ENDH, unsigned char _ENDL){
	char x;								//declaração de variável local 
	unsigned char DADO_I2C;				//declaração de variável local 
	x = I2C_LIVRE ();					//chamada à função com retorno de valor. Verifica se barramento está livre
	if (x == 0)							//se barramento ocupado, aborta transmissão e retorna
	{
		I2C_STOP();						//gera bit STOP
		return -1;						//erro na transmissão
	}
	else 								//barramento livre
	{
		I2C_START();					//barramento livre, gera condição START
		//TEMP = _ENDH << 1;				//rotaciona END_I2C para a esquerda - nao precisa, pq meu endereco ja ta certo
		I2C_TRANSMITE(_ENDH);			//transmite endereço
		if (!I2C_TESTA_ACK()) 			//se erro na transmismite endereço
		if (!I2C_TESTA_ACK()) 			//se erro na trassão, aborta transmissão
		{
			I2C_STOP();					//gera bit STOP
			return -1;					//erro na transmissão
 		}
		I2C_TRANSMITE(_ENDL);			//transmite endereço baixo
		if (!I2C_TESTA_ACK())			//se erro na transmissão, aborta transmissão
		{
			I2C_STOP();					//gera bit STOP
			return -1;					//erro na transmissão
		}
		I2C_RESTART();
        TEMP = _ENDH;                   // passa o endereço para o TEMP
		TEMP |= 0b00000001;				//insere bit R_W para leitura
		I2C_TRANSMITE(TEMP);			//transmite endereço
		if (!I2C_TESTA_ACK()) 			//se erro na transmissão, aborta transmissão
		{
			I2C_STOP();					//gera bit STOP
			return -1;					//erro na transmissão
 		}
		DADO_I2C = I2C_RECEBE();		//recebe dado
		I2C_NACK();						//gera bit NACK
		I2C_STOP();						//gera bit STOP
        
		return DADO_I2C;				//transmissão feita com sucesso
	}
}

void leituraHora(char *hour)
{
    char vet[] = {'0','1','2','3','4','5','6'};
    
    int j = 0;
    for(int i = 5;i >= 4; i--)
    {
        ENDL = i;
        hour[j] = vet[LEITURA_PCF8523T(ENDH, ENDL)>>4];
        j++;
        hour[j] = vet[LEITURA_PCF8523T(ENDH, ENDL)&0x0F];
        j++;
        if(i > 4)
            hour[j] = ':';
        j++;
    }
}