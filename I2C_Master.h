/********************************************************************
;	Nome do arquivo:		I2C_Master.h            
;	Data:				    10 de junho de 2010          
;	Versao:		  			1.0                              
;	MPLAB IDE:				v8.20a  
;	Autor:				    Wagner Zanco              
*********************************************************************/
/********************************************************************	
Esta biblioteca cont�m um conjunto de fun��es que permitem ao microcontrolador
se comunicar com qualquer dispositivo via I2C. O microcontrolador deve
estar configurado como dispositivo mestre. 
/********************************************************************/
#ifndef __I2C_MASTER_H
#define __I2C_MASTER_H
/********************************************************************/
// PCF8523
char buf [17];
unsigned char ENDH = 0b11010000; // Endereco para o PCF8523T
unsigned char ENDL = 1;
char DADO;
unsigned char TEMP;

char I2C_LIVRE  (void);
void I2C_START (void); 
void I2C_RESTART  (void);
void I2C_TRANSMITE  (unsigned char DADO_I2C);
char I2C_TESTA_ACK  (void);
void I2C_STOP  (void);
char I2C_ESCRITA  (unsigned char END_I2C, unsigned char DADO_I2C);
unsigned char I2C_RECEBE  (void);
void I2C_ACK  (void);
void I2C_NACK  (void);
unsigned char I2C_LEITURA (unsigned char END_I2C);
unsigned char ESCRITA_PCF8523T (unsigned char ENDH, unsigned char ENDL, char DADO);
unsigned char LEITURA_PCF8523T (unsigned char _ENDH, unsigned char _ENDL);
void leituraHora(char *hour);
//********************************************************************
#endif