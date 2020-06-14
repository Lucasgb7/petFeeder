/*
 * File:   main.c
 * Author: Jonath
 *
 * Created on June 4, 2020, 2:02 PM
 */


#include <xc.h>
#include "I2C_Master.h"

#pragma config MCLRE = ON, WDT = OFF, OSC = HS

#define _XTAL_FREQ 16000000

char  ESCRITA_PCF8523T  (unsigned char ENDH, unsigned char ENDL, char DADO);
char LEITURA_PCF8523T (unsigned char _ENDH, unsigned char _ENDL);
char Detecta_fim_escrita(void);

char buf [17];
unsigned char ENDH=0b11010000; //endereco para o pcf8523t
unsigned char ENDL=1;
char DADO;
unsigned char TEMP;

void main(void) {
    ADCON1 = 0x0f;  // todas as portas como digitais
    TRISC = 0xff;	//configura pinos SDA e SCL como entrada
	SSPCON1 = 0b00101000;				//configura dispositivo como mestre
										//Habilita porta serial e ativa pinos SDA E SCL <5>
										//ativa modo mestre <3:0>
	SSPADD = 3;						    //bit rate de 100kbps a Fosc = 16MHz ---- SSPADD = [(16M / 1M) / 4] -1
	SSPSTAT = 0b10000000;				//controle de inclina��o desativado <7>
										//n�veis de entrada conforme especifica��o I2C <6>
    
    DADO = 0x00; 
    ENDL= 3; //segundos
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);						//escreve dado na EEPROM
	__delay_ms(10);
    DADO = 0x00;
    ENDL= 4; //minutos
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);						//escreve dado na EEPROM
    DADO = 0x00;
    ENDL= 5; //horas
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);						//escreve dado na EEPROM
	__delay_ms(10);
    DADO = 0x12;
    ENDL= 6; //dia
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);						//escreve dado na EEPROM
	__delay_ms(10);
    DADO = 0x12;
    ENDL= 8; //mes
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);						//escreve dado na EEPROM
	__delay_ms(10);
    DADO = 0x12;
    ENDL= 9; //ano
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);						//escreve dado na EEPROM
	__delay_ms(10);
//    while(Detecta_fim_escrita()==-1)Detecta_fim_escrita ();	//aguarda final da escrita na EEPROM
//    __delay_ms(1);
    ENDL=3;
    DADO=LEITURA_PCF8523T (ENDH, ENDL);  //l� EEPROM
    //ENDL=0;
    //DADO= LEITURA_24C08(ENDH, ENDL);
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
    //    ENDL++;
    //DADO= LEITURA_24C08(ENDH, ENDL);
    //    ENDL++;
    //DADO= LEITURA_24C08(ENDH, ENDL);
    
    while(1);

    return;
}

char ESCRITA_PCF8523T (unsigned char ENDH, unsigned char ENDL, char DADO){
    char x;								//declara��o de vari�vel local 
	x = I2C_LIVRE ();					//chamada � fun��o com retorno de valor. Verifica se barramento est� livre
	if (x == 0)							//se barramento ocupado, aborta transmiss�o e retorna
	{
		I2C_STOP();						//gera bit STOP
		return -1;						//erro na transmiss�o
	}
	else 								//barramento livre
	{
		I2C_START();					//barramento livre, gera condi��o START
		//TEMP= ENDH << 1;				//rotaciona END_I2C para a esquerda (insere bit R_W para escrita)  - nao precisa, pq meu endereco ja ta certo
		I2C_TRANSMITE(ENDH);			//transmite endere�o alto
		if (!I2C_TESTA_ACK())	 		//se erro na transmiss�o, aborta transmiss�o
		{
			I2C_STOP();					//gera bit STOP
			return -1;					//erro na transmiss�o
 		}
		I2C_TRANSMITE(ENDL);			//transmite endere�o baixo
		if (!I2C_TESTA_ACK())			//se erro na transmiss�o, aborta transmiss�o
		{
			I2C_STOP();					//gera bit STOP
			return -1;					//erro na transmiss�o
		}
		I2C_TRANSMITE(DADO);			//transmite dado
		if (!I2C_TESTA_ACK())	 		//se erro na transmiss�o, aborta transmiss�o
		{
			I2C_STOP();					//gera bit STOP
			return -1;					//erro na transmiss�o
 		}
		I2C_STOP();						//gera bit STOP
        
        
		return DADO;					//transmiss�o feita com sucesso
	}
}

char LEITURA_PCF8523T (unsigned char _ENDH, unsigned char _ENDL){
	char x;								//declara��o de vari�vel local 
	unsigned char DADO_I2C;				//declara��o de vari�vel local 
	x = I2C_LIVRE ();					//chamada � fun��o com retorno de valor. Verifica se barramento est� livre
	if (x == 0)							//se barramento ocupado, aborta transmiss�o e retorna
	{
		I2C_STOP();						//gera bit STOP
		return -1;						//erro na transmiss�o
	}
	else 								//barramento livre
	{
		I2C_START();					//barramento livre, gera condi��o START
		//TEMP = _ENDH << 1;				//rotaciona END_I2C para a esquerda - nao precisa, pq meu endereco ja ta certo
		I2C_TRANSMITE(_ENDH);			//transmite endere�o
		if (!I2C_TESTA_ACK()) 			//se erro na transmismite endere�o
		if (!I2C_TESTA_ACK()) 			//se erro na trass�o, aborta transmiss�o
		{
			I2C_STOP();					//gera bit STOP
			return -1;					//erro na transmiss�o
 		}
		I2C_TRANSMITE(_ENDL);			//transmite endere�o baixo
		if (!I2C_TESTA_ACK())			//se erro na transmiss�o, aborta transmiss�o
		{
			I2C_STOP();					//gera bit STOP
			return -1;					//erro na transmiss�o
		}
		I2C_RESTART();
        TEMP = _ENDH;                   // passa o endere�o para o TEMP
		TEMP |= 0b00000001;				//insere bit R_W para leitura
		I2C_TRANSMITE(TEMP);			//transmite endere�o
		if (!I2C_TESTA_ACK()) 			//se erro na transmiss�o, aborta transmiss�o
		{
			I2C_STOP();					//gera bit STOP
			return -1;					//erro na transmiss�o
 		}
		DADO_I2C = I2C_RECEBE();		//recebe dado
		I2C_NACK();						//gera bit NACK
		I2C_STOP();						//gera bit STOP
        
		return DADO_I2C;				//transmiss�o feita com sucesso
	}
}