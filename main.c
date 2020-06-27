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
void writeString(const char *c)                  // Function to print Strings on LCD
{
    unsigned char aux = 0;
    while(*c != '\0')                           // While loop until the end of the string
    {
        if(aux >= 31)
            return;
        else if(aux == 15)
        {
            writeChar('-');
            sendCMD(0xC0);
            writeChar(*c);
            c++;
            aux++;
        }
        else
        {
            writeChar(*c);                       // Sending character by character
            c++;                               // Moving pointer to next character
            aux++;
        }
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
    TRISC = 0xFF;   // portas de entrada
    ADCON1 = 0x0F; // portas digitais
    
    // i2c
    SSPCON1 = 0b00101000;				// configura dispositivo como mestre
										// habilita porta serial e ativa pinos SDA E SCL <5>
										// ativa modo mestre <3:0>
	SSPADD = 3;						    // bit rate de 100kbps a Fosc = 16MHz ---- SSPADD = [(16M / 1M) / 4] -1
	SSPSTAT = 0b10000000;				// controle de inclinação desativado <7>
										// níveis de entrada conforme especificação I2C <6>
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
// display fim

char ESCRITA_PCF8523T  (unsigned char ENDH, unsigned char ENDL, char DADO);
char LEITURA_PCF8523T (unsigned char _ENDH, unsigned char _ENDL);

// PCF8523
char buf [17];
unsigned char ENDH = 0b11010000; // Endereco para o PCF8523T
unsigned char ENDL = 1;
char DADO;
unsigned char TEMP;

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

// Funcoes de configuracao do petfeeder
void configuraHorario(char hora[], char minuto[]){
    int hora_I, minuto_I;
    hora_I = atoi(hora); // converte string para inteiro
    minuto_I = atoi(minuto);
    
    gotoxy(0, 0); 
    writeString("Informe");
    gotoxy(0, 1); 
    writeString("horario:");
    gotoxy(0, 3);
    writeString(hora);
    gotoxy(2, 3);
    writeChar(':');
    gotoxy(3, 3);
    writeString(minuto);
   
    int botaoUP = 0;
    int botaoDOWN = 0;
    
    //horas
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
            gotoxy(0, 3);
            writeString(hora);
            gotoxy(2, 3);
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
            gotoxy(0, 3);
            writeString(hora);
            gotoxy(2, 3);
            writeChar(':');
        }
    }
    
    __delay_ms(500);
    
    //minutos
    while(!BTN_OK){
        if(!BTN_UP) botaoUP = 0;
        if(BTN_UP && botaoUP == 0){
            botaoUP = 1;
            minuto_I += 1;
            if(minuto_I > 59) minuto_I = 0;
            if(minuto_I < 0) minuto_I = 59;
            itoa(minuto_I, minuto, 10); // converte inteiro para string
            if(hora_I < 10){
                minuto[1] = minuto[0];
                minuto[0] = '0';    // ex: 09, 08, 07...
            }
            gotoxy(2, 3);
            writeChar(':');
            gotoxy(3, 3);
            writeString(minuto);
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
            gotoxy(2, 3);
            writeChar(':');
            gotoxy(3, 3);
            writeString(minuto);
        }
    }
    
    __delay_ms(500);
}

int printConfirmacao(){
    gotoxy(0, 0); 
    writeString("Adicionar outro");
    gotoxy(0, 1); 
    writeString("horario?");
    gotoxy(0, 2); 
    writeString("OK = SIM");
    gotoxy(0, 3); 
    writeString("<< = NAO");

    while(1){
        if(BTN_OK)
            return 1;   // configurar outro horario
        else
            return 0;   // proximo passo
    }
    
    __delay_ms(500);
}

int configuraQuantidade(){
    int quantidade = 1;
    char quantidade_c[2];
    itoa(quantidade,quantidade_c,10);
    
    gotoxy(0, 0); 
    writeString("Quantidade:");
    gotoxy(0, 1);
    writeString(quantidade_c);
    
    int botaoUP = 0;
    int botaoDOWN = 0;
    
    while(!BTN_OK){
        if(!BTN_UP) botaoUP = 0;
        if(BTN_UP && botaoUP == 0){
            botaoUP = 1;
            quantidade += 1;
            if(quantidade > 10) quantidade = 0;
            if(quantidade < 0) quantidade = 10;
            itoa(quantidade,quantidade_c,10);
            if(quantidade < 10){
                quantidade_c[1] = quantidade_c[0];
                quantidade_c[0] = '0';
            }
            gotoxy(0, 1);
            writeString(quantidade_c);
        }
        if(!BTN_DOWN) botaoDOWN = 0;
        if(BTN_DOWN && botaoDOWN == 0){
            botaoDOWN = 1;
            quantidade -= 1;
            if(quantidade > 10) quantidade = 0;
            if(quantidade < 0) quantidade = 10;
            itoa(quantidade,quantidade_c,10);
            if(quantidade < 10){
                quantidade_c[1] = quantidade_c[0];
                quantidade_c[0] = '0';
            }
            gotoxy(0, 1);
            writeString(quantidade_c);
        }
    }
    
    __delay_ms(500);
    
    return quantidade;
}

void configuraHorarioAtual(char *hora[], char *minuto[], char *segundo[]){
    int hora_I, minuto_I, segundo_I;
    hora_I = atoi(hora); // converte string para inteiro
    minuto_I = atoi(minuto);
    segundo_I = atoi(segundo);
    
    gotoxy(0, 0); 
    writeString("Informe");
    gotoxy(0, 1); 
    writeString("horario:");
    gotoxy(0, 3);
    writeString(hora);
    gotoxy(2, 3);
    writeChar(':');
    gotoxy(3, 3);
    writeString(minuto);
    gotoxy(5, 3);
    writeChar(':');
    gotoxy(6, 3);
    writeString(segundo);
   
    int botaoUP = 0;
    int botaoDOWN = 0;   
    //horas
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
            gotoxy(0, 3);
            writeString(hora);
            gotoxy(2, 3);
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
            gotoxy(0, 3);
            writeString(hora);
            gotoxy(2, 3);
            writeChar(':');
        }
    }    
    __delay_ms(500);
    //minutos
    while(!BTN_OK){
        if(!BTN_UP) botaoUP = 0;
        if(BTN_UP && botaoUP == 0){
            botaoUP = 1;
            minuto_I += 1;
            if(minuto_I > 59) minuto_I = 0;
            if(minuto_I < 0) minuto_I = 59;
            itoa(minuto_I, minuto, 10); // converte inteiro para string
            if(hora_I < 10){
                minuto[1] = minuto[0];
                minuto[0] = '0';    // ex: 09, 08, 07...
            }
            gotoxy(2, 3);
            writeChar(':');
            gotoxy(3, 3);
            writeString(minuto);
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
            gotoxy(2, 3);
            writeChar(':');
            gotoxy(3, 3);
            writeString(minuto);
        }
    }
     __delay_ms(500);
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
                segundo_I[1] = segundo_I[0];
                segundo_I[0] = '0';    // ex: 09, 08, 07...
            }
            gotoxy(5, 3);
            writeChar(':');
            gotoxy(6, 3);
            writeString(segundo_I);
        }
        if(!BTN_DOWN) botaoDOWN = 0;
        if(BTN_DOWN && botaoDOWN == 0){
            botaoDOWN = 1;
            segundo_I -= 1;
            if(segundo_I > 59) segundo_I = 0;
            if(segundo_I < 0) segundo = 59;
            itoa(segundo_I,segundo,10);
            if(segundo_I < 10){
                segundo_I[1] = segundo[0];
                segundo_I[0] = '0';
            }
            gotoxy(5, 3);
            writeChar(':');
            gotoxy(6, 3);
            writeString(minuto);
        }
    }
    __delay_ms(500);
}

// nao funciona
unsigned char converteBinario(char str_decimal){
    switch (str_decimal){
        case '0':
            return 0b00000000;
            break;
    }
}

void main(void) {
    setup(); // configuracoes iniciais
    start(); // inicía o LCD
    pLCD = &PORTD;  // LCD le a porta D diretamente para configuracoes iniciais
    
    gotoxy(0, 0); 
    writeString("Iniciando");
    gotoxy(0, 1);
    writeString("dispositivo...");
     __delay_ms(1000);   
    sendCMD(CLEAR);
    
    char horaAtual[2] = "00";
    char minutoAtual[2] = "00";
    char segundoAtual[2] = "00";
    do{
        configuraHorarioAtual(&horaAtual, &minutoAtual, &segundoAtual);
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
    }while(1);
    
    char hora[2] = "00";
    char minuto[2] = "00";
    do{
        configuraHorario(hora, minuto);
        // adicionar a hora e minuto para o alarme do PCF

    }while(printConfirmacao());
    
    sendCMD(CLEAR);
    int quantidade = configuraQuantidade();
    
    while(1){}      // Loop
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

// Funcao de leitura do PCF8523T
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