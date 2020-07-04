/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#ifndef __EEPROM_H
#define	__EEPROM_H

unsigned char readEEPROM(unsigned char address);
void writeEEPROM(unsigned char address, unsigned char data);
void armazenaAlarmeEEPROM(char tabelaAlarmes[][4], int quantidadeAlarmes);
void carregaAlarmeEEPROM(char tabelaAlarmes[][4], int quantidadeAlarmes);

#endif	/* XC_HEADER_TEMPLATE_H */

