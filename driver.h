#include <8051.h>
#include <stdio.h>
#include "reg832.h"

#define e	P0_2
#define rs	P0_3
#define lcdport		P0		//SELECTIE POORT 0

#define cleardisp	0x01	//LEEG MAKEN VAN HET SCHERM
#define cursathom	0x02	//CURSOR OP DE EERSTE PLAATS ZETTEN

#define ENTRYMODE   0x06	//00000110B            ;CURSOR NAAR RECHTS, SCHERM VAST
#define DISPLAYON	0x01	//00000001B            ;HOMEN EN CLEAR CURSOR
#define DISPLAYOF	0x08	//00001000B            ;DISPLAY OFF, CURSOR OFF
#define CURSOROFF	0x0C	//00001100B            ;DISPLAY ON ZONDER CURSOR
#define CURSORONB	0x0F	//00001111B            ;CURSOR ON AND BLINK
#define CURSORONN	0x0E	//00001110B            ;CURSOR ON NO BLINK
#define FUNCTIONS	0x28	// 00101000B            ;INTERFACE LENGTE EN KARAKTER FONT
#define CGRAM		0x40	//01000000B            ;SELECTIE KARAKTER GENERATOR RAM
#define DDRAM		0x80	//10000000B            ;SELECTIE DATA DISPLAY RAM

void wait (int time);

void lcdnibd (unsigned char c);

void lcdd (unsigned char c);

void lcdmsg (unsigned char * str);

void lcdnibc (unsigned char c);

void lcdc (unsigned char c);

void lcdinit (void);

// SerInit stelt de seriële poort in op 9600 baud
void serinit (void);

// putchar functie nodig voor printf
void putchar(char c);
