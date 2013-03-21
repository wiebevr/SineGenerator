#include <stdio.h>
#include "reg832.h"
#include "lcd.h"

char g_flags;
char g_dds_inc = 1; // Initialize at 0.1Hz


#define TIMER_FLAG 0x01
#define LCD_FLAG 0x02
#define UART_FLAG 0x04
#define ADC_FLAG 0x08

#define ADC_TIMER_RELOAD 64138
#define DSS_TIMER_RELOAD 62125

#define NUM_ADC_HISTORY_VALUES 8
static unsigned short g_adc_value = 0;
static unsigned short g_adc_history[NUM_ADC_HISTORY_VALUES];

unsigned short read_adc();

// DDS data:
extern const char dac_high[];
extern const char dac_low[];


// Init stuff:
void init_adc();
void init_interrupts();
void init_dac();

// Background routines:
void update_timer();
void update_lcd();
void update_uart();
void update_adc();

// Interrupts:
// 1 ms timer interrupt to update the MA array
void adc_interrupt() interrupt 1;
void dds_interrupt() interrupt 3;


void main(void)
{
    /* Set the clock rate to 16 MHz */
    PLLCON = 0x00;

	init_adc();
    init_dac();
    init_interrupts();
	InitializeLCD();
	SetCursorOff();

    while (1)
    {
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


void init_interrupts()
{
    // Enable all interrupts.
    EA = 1;
    // Enable timer interrupts
    IEIP2 = 0x04;
    // Enable timer 0 interrupts.
    ET0 = 1;
    //
    // Enable timer 0 interrupts.
    ET1 = 1;

    // Timers:
    // 16 Bit prescaler on both timers
    TMOD = 0x11;
    // Enable timer 0
    TR0 = 1;
    // Enable timer 1
    TR1 = 1;
}

void update_timer()
{
    // TODO: Implement 
}

void update_lcd()
{
	char buffer[6] = "00000";
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
// ---------------------------------------------------------------------- 
// DDS Stuff:
void init_dac()
{
    DACCON = 0x7D;
}

void dds_interrupt() interrupt 3
{
    static short it = 0;
    it += g_dds_inc;
    if (it > 4095)
    {
        it -= 4095;
    }

    // Reload the timer:
    TH0 = (DDS_TIMER_RELOAD & 0xFF00)>>8;
    TL0 = (DDS_TIMER_RELOAD & 0x00FF);

    DAC0H = dac_high[it];
    DAC0L = dac_high[it];

    
}

// ---------------------------------------------------------------------- 
// ADC Stuff:
void init_adc()
{
	ADCCON1 = 0xDC;   //11011100
	ADCCON2 = 0x27;	  //00000111
}

void adc_interrupt(void) interrupt 1
{
    static char it = 0;

    // Reload the timer:
    TH0 = (ADC_TIMER_RELOAD & 0xFF00)>>8;
    TL0 = (ADC_TIMER_RELOAD & 0x00FF);

    g_adc_history[it++] = read_adc();
    // Limit it to 0-7
    it &= 0x07;
    g_flags |= ADC_FLAG;
}

unsigned short read_adc()
{
	// Return 12 most significant bits
	return ((ADCDATAH & 0x0f) << 8) | ADCDATAL;
}

void update_adc()
{
    unsigned short adc_value = 0;
    int i;

    for (i = 0; i < NUM_ADC_HISTORY_VALUES; ++i)
    {
        adc_value += g_adc_history[i];
    }
    // Divide by eight
    g_adc_value = adc_value>>3;
    g_dds_inc = g_adc_value>>2;
    if (g_dds_inc == 0)
    {
        g_dds_inc = 0;
    }
    else if (g_dds_inc < 999)
    {
        g_dds_inc = 999;
    }
}

