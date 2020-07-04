/********************************************************************
;	Nome do arquivo:		I2C_Master.c            
;	Data:				    10 de maio de 2010          
;	Versao:		  			1.0                              
;	MPLAB IDE:				v8.20a  
;	Autor:				    Wagner Zanco              
*********************************************************************/
/********************************************************************	
Esta biblioteca contém um conjunto de funções que permitem ao microcontrolador
se comunicar com qualquer dispositivo via I2C. O microcontrolador deve
estar configurado como dispositivo mestre. 
//********************************************************************/
#include <xc.h>					//diretiva de compilação
#include "I2C_Master.h"					//diretiva de compilação
/********************************************************************/
char   I2C_LIVRE  (void)				//função I2C_LIVRE
{
	if(SSPSTATbits.R_W) return 0;		//retorna 0 se existe transmissão em andamento
		if(SSPCON2 & 0x1F) return 0;	//retorna 0 se existe algum evento de transmissão em andamento
			else return 1;  			//retorna 1 se barramento está livre
}//*********************************************************************
void I2C_START (void) 					//função I2C_START
{
	SSPCON2bits.SEN = 1;				//inicia a condição START	
	while (SSPCON2bits.SEN);			//aguarda terminar a condição START	
}//*********************************************************************
void   I2C_RESTART  (void)				//função I2C_RESTART
{
	SSPCON2bits.RSEN = 1;				//inicia a condição RE-START
	while (SSPCON2bits.RSEN);			//aguarda terminar a condição RE-START	
}//*********************************************************************
void I2C_TRANSMITE  (unsigned char DADO_I2C)	//função I2C_TRANSMITE
{
	SSPBUF = DADO_I2C;					//carrega dado a ser transmitido no registrador SSPBUF
	while (SSPSTATbits.BF);				//aguarda buffer esvaziar
	while (SSPSTATbits.R_W);			//aguarda transmissão terminar
}//*********************************************************************
char I2C_TESTA_ACK  (void)				//função I2C_TESTA_ACK
{
	if (!SSPCON2bits.ACKSTAT) return 1;	//escravo recebeu dado com sucesso
		else return 0;					//erro na transmissão
}//*********************************************************************
void  I2C_STOP  (void)					//função I2C_STOP
{
	SSPCON2bits.PEN = 1;				//inicia a condição STOP
	while (SSPCON2bits.PEN);			//aguarda terminar condição STOP
}//*********************************************************************
//esta função transmite um byte via I2C com endereçamento de 7 bits 
char  I2C_ESCRITA  (unsigned char END_I2C, unsigned char DADO_I2C)	//função I2C_ESCRITA
{
char x;									//declaração de variável local 
	x = I2C_LIVRE ();					//chamada à função com retorno de valor. Verifica se barramento está livre
	if (x == 0)							//se barramento ocupado, aborta transmissão e retorna
	{
		I2C_STOP();						//gera bit STOP
		return -1;						//erro na transmissão
	}
	else 								//barramento livre
	{
		I2C_START();					//barramento livre, gera condição START
		END_I2C <<= 1;					//rotaciona END_I2C para a esquerda (insere bit R_W para escrita) 
		I2C_TRANSMITE(END_I2C);			//transmite endereço
		if (!I2C_TESTA_ACK())	 		//se erro na transmissão, aborta transmissão
		{
			I2C_STOP();					//gera bit STOP
			return -1;					//erro na transmissão
 		}
		I2C_TRANSMITE(DADO_I2C);		//transmite dado
		if (!I2C_TESTA_ACK())			//se erro na transmissão, aborta transmissão
		{
			I2C_STOP();					//gera bit STOP
			return -1;					//erro na transmissão
		}
		I2C_STOP();						//gera bit STOP
		return 0;						//transmissão feita com sucesso
	}
}//*********************************************************************
unsigned char I2C_RECEBE  (void)		//função I2C_RECEBE
{
unsigned char x;
	SSPCON2bits.RCEN = 1;				//ativa mestre-receptor
	while (SSPCON2bits.RCEN);			//aguarda chegada do dado
	while (!SSPSTATbits.BF);			//aguarda chegada do dado
	Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    Nop();
    
	x = SSPBUF;
	return x;							//retorna dado		
}//*********************************************************************
void I2C_ACK  (void)					//função I2C_ACK
{
	SSPCON2bits.ACKDT = 0;				//carrega bit ACK
	SSPCON2bits.ACKEN = 1;				//inicia seqüência ACK		
	while (SSPCON2bits.ACKEN);			//aguarda terminar seqüência ACK	
}//*********************************************************************
void I2C_NACK  (void)					//função I2C_NACK
{
	SSPCON2bits.ACKDT = 1;				//carrega bit NACK
	SSPCON2bits.ACKEN = 1;				//inicia seqüência ACK		
	while (SSPCON2bits.ACKEN);			//aguarda terminar seqüência ACK	
}//*********************************************************************
//Esta função efetua a leitura de um byte via barramento I2C com endereçamento de 7 bits. 
unsigned char I2C_LEITURA (unsigned char END_I2C)	//função I2C_LEITURA
{
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
		END_I2C <<= 1;					//rotaciona END_I2C para a esquerda 
		END_I2C &= 0b00000001;			//insere bit R_W para leitura
		I2C_TRANSMITE(END_I2C);			//transmite endereço
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

void leituraHora(char *hour){
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
//*********************************************************************
