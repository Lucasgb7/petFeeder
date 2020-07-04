/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#ifndef __LCD_H
#define	__LCD_H

#define _XTAL_FREQ 16000000
#define CLEAR   0x01

// define display
typedef struct tDisplayPort{
    char RS: 1;
    char R_W: 1;
    char E: 1;
    char lixo: 1;
    char NData: 4;
};
struct tDisplayPort * pLCD;

void pulseEnable();
void waitIdle();
void sendCMD(char data);
void writeChar(char data);
void writeString(char c[], int tam);
void gotoxy(char x, char y);
void enable();
void start();

#endif