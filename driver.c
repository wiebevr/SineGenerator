#include "driver.h"

void wait (int time) {
	int i;
	for (i=0;i<time;i++);	// time keer niets doen.
}

void lcdnibd (unsigned char c){
	rs=1;
	e=0;
	c&=0xF0;
	c|=0x0b;
	P0=c;
	e=1;
	e=1;
	e=0;
	wait(100);
}

void lcdd (unsigned char c){
	lcdnibd(c);
	c=c<<4;
	lcdnibd(c);
}

void lcdmsg (unsigned char * str) {
	for(;*str!=0;str++)
		lcdd(*str);
}

void lcdnibc (unsigned char c){
	rs=0;
	e=0;
	c&=0xF0;
	c|=0x03;
	P0=c;
	e=1;
	e=1;
	e=0;
	wait(1000);
}

void lcdc (unsigned char c){
	lcdnibc(c);
	c=c<<4;
	lcdnibc(c);
}

void lcdinit (void) {
	P0=0x03;
	wait(100);
	lcdnibc(0x30);
	lcdnibc(0x30);
	lcdnibc(0x30);
	lcdnibc(0x20);
	lcdc(FUNCTIONS);
	lcdc(CURSORONB);
	lcdc(DISPLAYON);
	lcdc(ENTRYMODE);
}

// SerInit stelt de seriële poort in op 9600 baud
void serinit (void) {
	if ((PLLCON & 0x07) <= 5) {
		//hier mogen we alleen komen als de waarde van de PLL bruikbaar is
		T3FD = 0x2D; //zie databoek t3 als baud rate generator
		T3CON = (((~PLLCON)-2) & 0x07)+0x80; //baudrate instellen afhankelijk van pll
		SCON = 0x50;	//Uart initializeren
	}
}

// putchar functie nodig voor printf
void putchar(char c) {
	TI=0;	
    SBUF=c;
	while(!TI); //wachten tot het karakter is verzonden
}
