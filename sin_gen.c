#include "lcd.h"

char g_flags;
#define TIMER_FLAG 0x01
#define LED_FLAG 0x02
#define UART_FLAG 0x04

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
    while (1)
    {
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
