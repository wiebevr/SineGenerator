#include "reg832.h"
#include "lcd.h"
#include <stdio.h>

char g_flags;
#define TIMER_FLAG 0x01
#define LCD_FLAG 0x02
#define UART_FLAG 0x04
#define ADC_FLAG 0x08
static unsigned short g_adc_value = 0;

unsigned short read_adc();
void init_adc();
void update_timer();
void update_lcd();
void update_uart();
void update_adc();

void main(void)
{
	init_adc();
	InitializeLCD();
    while (1)
    {
		g_adc_value = read_adc();
		update_lcd();
        if (g_flags & TIMER_FLAG)
        {
            update_timer();
            g_flags &= ~TIMER_FLAG;
        }
        if (g_flags & LCD_FLAG)
        {
            update_lcd();
            g_flags &= ~LCD_FLAG;
        }
        if (g_flags & UART_FLAG)
        {
            update_uart();
            g_flags &= ~UART_FLAG;
        }
		if (g_flags & ADC_FLAG)
        {
            update_adc();
            g_flags &= ~ADC_FLAG;
        }
    }
}

void init_adc()
{
	ADCCON1 = 0xDC;   //11011100
	ADCCON2 = 0x27;	  //00000111
}
unsigned short read_adc()
{
	// Return 12 most significant bits
	return ((ADCDATAH & 0x0f) << 8) | ADCDATAL;
}

void update_timer()
{
    // TODO: Implement 
}

void update_lcd()
{
	char buffer[6] = "00000";
    ClearScreen();
	WriteStringAtPos(0,0,"ADC:");
	sprintf(buffer, "%4.4d", (unsigned short)(g_adc_value*1.221));
	buffer[2] = buffer[1];buffer[3] = buffer[2];
	buffer[4] = 'V'; buffer[1] = '.';
	WriteStringAtPos(0,10,buffer);
	WriteStringAtPos(1,0,"Sinus:");
	WriteStringAtPos(1,10,buffer);
}

void update_uart()
{
    // TODO: Implement 
}

void update_adc()
{
	//moving average nog doen
    g_adc_value = read_adc(); 
}

