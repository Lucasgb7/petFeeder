/*
 * File:   main.c
 * Author: Jonath e Lucas
 *
 * Created on June 4, 2020, 2:02 PM
 */


#include <xc.h>
#include "I2C_Master.h"

#pragma config MCLRE = ON, WDT = OFF, OSC = HS
#define _XTAL_FREQ 16000000

//define display
#define CLEAR   0x01

//  comandos do display
typedef struct tDisplayPort{
    char RS: 1;
    char R_W: 1;
    char E: 1;
    char lixo: 1;
    char NData: 4;
};
struct tDisplayPort * pLCD;

// Ativa e desativa o Enable para um pulso
void pulseEnable(){ 
    __delay_us(1000);
    pLCD->E = 1;
    __delay_us(1000);
    pLCD->E = 0;
}
// 
void waitIdle(){
    char aux = 0xFF;
    TRISD = 0xF0;
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
// Função para enviar o cursor
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

void writeString(char data[], int tam){
    for(int i = 0; i < tam; i++){
        writeChar(data[i]);
    }
}

// Envia o cursor para alguma posição do LCD
void gotoxy(char x, char y){
    if ((x < 15 || x > 0) && (y < 3 || y > 0)){ // Verifica se tá dentro do tamanho normal
        if(y == 0){
            sendCMD(0x80 + x);  // N = 0 endereço inicial: 0x80
        }else if(y == 1){
            sendCMD(0xC0 + x); // N = 1 endereço inicial: 0xC0
        }else if(y == 2){
            sendCMD(0x90 + x); // N = 1 endereço inicial: 0xC0
        }else{
            sendCMD(0xD0 + x); // N = 1 endereço inicial: 0xC0
        }
    } else {
        sendCMD(CLEAR);
    }
}
// Configuração do microcontrolador
void setup(){
    TRISD = 0x00;
}
// Pulso direto na porta D
void enable(){
    __delay_ms(1);
    PORTD = PORTD|0x04; // Pulso do Enable = 1 (Valor + 0000 0100)
    __delay_ms(1);
    PORTD = PORTD&0xFB;  // Pulso do Enable = 0 (Valor + 1111 1011)
    __delay_ms(5);
}
// Configurações iniciais
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
// display fim

char ESCRITA_PCF8523T  (unsigned char ENDH, unsigned char ENDL, char DADO);
char LEITURA_PCF8523T (unsigned char _ENDH, unsigned char _ENDL);

// PCF8523
char buf [17];
unsigned char ENDH=0b11010000; //endereco para o pcf8523t
unsigned char ENDL=1;
char DADO;
unsigned char TEMP;

void main(void) {
    setup();
    start();
    pLCD = &PORTD;  // LCD lê a porta D diretamente para configurações iniciais
    gotoxy(0, 0); 
    writeChar('A');
    writeChar('B');
    //writeString('abc', 3);
    while(1){}
    return;
    
    /*
    ADCON1 = 0x0f;                      // todas as portas como digitais
    TRISC = 0xff;                       //configura pinos SDA e SCL como entrada
	SSPCON1 = 0b00101000;				//configura dispositivo como mestre
										//Habilita porta serial e ativa pinos SDA E SCL <5>
										//ativa modo mestre <3:0>
	SSPADD = 3;						    //bit rate de 100kbps a Fosc = 16MHz ---- SSPADD = [(16M / 1M) / 4] -1
	SSPSTAT = 0b10000000;				//controle de inclinação desativado <7>
										//níveis de entrada conforme especificação I2C <6>
    
    TRISD = 0x00; // Definindo como porta de saida
    TRISB = 0xFF; // Definindo como porta de entrada
    
    DADO = 0x00; 
    ENDL= 3; //segundos
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);
	__delay_ms(10);
    DADO = 0x00;
    ENDL= 4; //minutos
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);
    DADO = 0x00;
    ENDL= 5; //horas
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);
	__delay_ms(10);
    DADO = 0x12;
    ENDL= 6; //dia
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);
	__delay_ms(10);
    DADO = 0x12;
    ENDL= 8; //mes
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);
	__delay_ms(10);
    DADO = 0x12;
    ENDL= 9; //ano
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);
	__delay_ms(10);

    ENDL=3;
    DADO=LEITURA_PCF8523T (ENDH, ENDL);
    ENDL = 4;
    DADO= LEITURA_PCF8523T(ENDH, ENDL);
    ENDL = 5;
    DADO= LEITURA_PCF8523T(ENDH, ENDL);
    ENDL = 6;
    DADO= LEITURA_PCF8523T(ENDH, ENDL);
    ENDL = 8;
    DADO= LEITURA_PCF8523T(ENDH, ENDL);
    ENDL = 9;
    DADO= LEITURA_PCF8523T(ENDH, ENDL);
    
    while(1);

    return;
    */
}

char ESCRITA_PCF8523T (unsigned char ENDH, unsigned char ENDL, char DADO){
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

char LEITURA_PCF8523T (unsigned char _ENDH, unsigned char _ENDL){
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