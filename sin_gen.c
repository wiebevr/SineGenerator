#include "lcd.h"
#include "reg832.h"

char g_flags;
#define TIMER_FLAG 0x01
#define LCD_FLAG 0x02
#define UART_FLAG 0x04

char read_adc();
void init_adc();

void update_timer()
{
    // TODO: Implement 
}

void update_lcd()
{
    // TODO: Implement 
}

void update_uart()
{
    // TODO: Implement 
}


void main(void)
{
	init_adc();
    while (1)
    {
		P2 = read_adc();
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
    }
}

void init_adc()
{
	ADCCON1 = 0xDC;   //11011100
	ADCCON2 = 0x27;	  //00000111
}
char read_adc()
{
	char value;
	// Return 8 most significant bits
	value = ADCDATAH << 4;
	return value | ((ADCDATAL & 0xf0) >> 4);
}
