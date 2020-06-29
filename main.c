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

//void unsignedCharToChar(unsigned char *a, char b[], int tam){
//    for(int i=0; i < tam; i++){
//        b[i] = *a;
//        a++;
//    }
//}

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
    
    //horas
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
    
    //minutos
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

void exibeInformacoes(char horaAtual[], char minutoAtual[], char proxHora[], char proxMinuto[], char ultHora[], char ultMinuto[]){
    gotoxy(0, 0); 
    writeString("Horario: ", 10); 
    writeString(horaAtual, 2);
    writeChar(':');
    writeString(minutoAtual, 2);
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

void setAlarme(char hora[], char minuto[]){
    int horaInt = (int)strtol(hora, NULL, 16);
    ENDL = 11; //hora alarme
    ESCRITA_PCF8523T(ENDH, ENDL, horaInt);
    
    int minutoInt = (int)strtol(minuto, NULL, 16);
    ENDL = 10; //minuto alarme
    ESCRITA_PCF8523T(ENDH, ENDL, minutoInt);
}

void ordenaMatriz(char matriz[][4]){
    for(int i=0; i < 3; i++){
        for(int j=0; j < 3; j++){
            int auxI = atoi(matriz[i]);
            int auxJ = atoi(matriz[j]);
            if(auxJ < auxI){
                char bixoNaoSei;
            }
        }
    }
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
    int horaAtualInt = (int)strtol(horaAtual, NULL, 16);
    int minutoAtualInt = (int)strtol(minutoAtual, NULL, 16);
    int segundoAtualInt = (int)strtol(segundoAtual, NULL, 16);
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
    ESCRITA_PCF8523T(ENDH, ENDL, segundoAtualInt);						//escreve dado na EEPROM
	__delay_ms(10);
    ENDL= 4; //minutos
    ESCRITA_PCF8523T(ENDH, ENDL, minutoAtualInt);						//escreve dado na EEPROM
    ENDL= 5; //horas
    ESCRITA_PCF8523T(ENDH, ENDL, horaAtualInt);						//escreve dado na EEPROM
	__delay_ms(10);
    
    char horaAlarme[2] = "00";
    char minutoAlarme[2] = "00";
    char tabelaAlarmes[3][4];
    int quantidadeAlarmes = 0;
    
    do{
        sendCMD(CLEAR);
        configuraAlarme(horaAlarme, minutoAlarme);
        tabelaAlarmes[quantidadeAlarmes][0] = horaAlarme[0];
        tabelaAlarmes[quantidadeAlarmes][1] = horaAlarme[1];
        tabelaAlarmes[quantidadeAlarmes][2] = minutoAlarme[0];
        tabelaAlarmes[quantidadeAlarmes][3] = minutoAlarme[1];
        quantidadeAlarmes += 1;
        horaAlarme[0] = '0';
        horaAlarme[1] = '0';
        minutoAlarme[0] = '0';
        minutoAlarme[1] = '0';

    }while(printConfirmacao() && quantidadeAlarmes < 3);
    
    sendCMD(CLEAR);
    int quantidade = configuraQuantidade();
    
    sendCMD(CLEAR);
    int som = configuraSom();
    
//    sendCMD(CLEAR);
//    ordenaMatriz(tabelaAlarmes);
    
    DADO = 0b00000010; // Ativa a interrupcao por alarme
    ENDL = 0; // Register Control_1
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);
    DADO = 0b00001000; // Seta flag quando alarme é disparado, limpa flag quando não há interrupção
    ENDL = 1; // Register Control_2
    ESCRITA_PCF8523T(ENDH, ENDL, DADO);
    
    int alarmeAtual = 0;
    
    char minutoAlarmeDispositivo[2];
    minutoAlarmeDispositivo[0] = tabelaAlarmes[alarmeAtual][2];
    minutoAlarmeDispositivo[1] = tabelaAlarmes[alarmeAtual][3];
    
    char horaAlarmeDispositivo[2];
    horaAlarmeDispositivo[0] = tabelaAlarmes[alarmeAtual][0];
    horaAlarmeDispositivo[1] = tabelaAlarmes[alarmeAtual][1];
    
    char proxAlarmeHora[2];
    proxAlarmeHora[0] = tabelaAlarmes[alarmeAtual+1][2];
    proxAlarmeHora[1] = tabelaAlarmes[alarmeAtual+1][3];
    
    char proxAlarmeMinuto[2];
    proxAlarmeMinuto[0] = tabelaAlarmes[alarmeAtual+1][0];
    proxAlarmeMinuto[1] = tabelaAlarmes[alarmeAtual+1][1];
    
    setAlarme(horaAlarmeDispositivo, minutoAlarmeDispositivo);
    
    PR2 = 199;                //periodo de 5000Hz para um oscilador de 16MHz
    CCPR1L = 0;               //Duty Cycle de 50%
    CCP1CONbits.DC1B = 0;     //dois bits menos significativos como zero
    CCP1CONbits.CCP1M = 0x0C; //configura modulo CCP para operar como PWM (00001100)
    T2CON = 0x05;             //Prescaler TMR2 como 1:4 ativando timer 2
    
    sendCMD(CLEAR);
    
    while(1){
        char dadoHoraAtual[2];
        char dadoMinutoAtual[2];
        char dadoMinuto;
//        ENDL=5;
//        unsignedCharToChar(LEITURA_PCF8523T(ENDH, ENDL), dadoHora, 2);
        
//        ENDL=4;
//        unsignedCharToChar(LEITURA_PCF8523T(ENDH, ENDL), dadoMinuto, 2);
        if(dadoMinutoAtual[1] != dadoMinuto){
            exibeInformacoes(dadoHoraAtual, dadoMinutoAtual, horaAlarmeDispositivo, minutoAlarmeDispositivo, proxAlarmeHora, proxAlarmeMinuto);
            dadoMinuto = dadoMinutoAtual[1];
        }
        if(PORTBbits.RB4){
            despejarRacao(som, quantidade);
            paraDespejar();
            
            alarmeAtual += 1;
            if(alarmeAtual == 3)
                alarmeAtual = 0;

            horaAlarmeDispositivo[0] = tabelaAlarmes[alarmeAtual][0];
            horaAlarmeDispositivo[1] = tabelaAlarmes[alarmeAtual][1];
            
            minutoAlarmeDispositivo[0] = tabelaAlarmes[alarmeAtual][2];
            minutoAlarmeDispositivo[1] = tabelaAlarmes[alarmeAtual][3];

            setAlarme(horaAlarmeDispositivo, minutoAlarmeDispositivo);
            
            proxAlarmeHora[0] = tabelaAlarmes[alarmeAtual+1][2];
            proxAlarmeHora[1] = tabelaAlarmes[alarmeAtual+1][3];
            
            proxAlarmeMinuto[0] = tabelaAlarmes[alarmeAtual+1][0];
            proxAlarmeMinuto[1] = tabelaAlarmes[alarmeAtual+1][1];
        }
        if(BTN_OK){
            despejarRacao(som, quantidade);
            paraDespejar();
        }
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