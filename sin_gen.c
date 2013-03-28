#include <stdio.h>
#include <string.h>
#include "reg832.h"
#include "lcd.h"

char g_flags;
unsigned short g_dds_inc = 1; // Initialize at 0.1Hz


#define TIMER_FLAG 0x01
#define LCD_FLAG 0x02
#define UART_FLAG 0x04
#define ADC_FLAG 0x08

#define ADC_TIMER_RELOAD 64138
//#define DDS_TIMER_RELOAD 62125
#define DDS_TIMER_RELOAD 42

#define NUM_ADC_HISTORY_VALUES 8

// Errors on leds:
static unsigned char g_uart_error_status;
#define NO_ERROR 0
#define SYNTAX_ERROR 1
#define TO_LONG_ERROR 2

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
void init_uart();

// Background routines:
void update_timer();
void update_lcd();
void answer_uart();
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
    init_uart();
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
        //if (g_flags & UART_FLAG)
        if(RI)
        {
            answer_uart();
            //g_flags &= ~UART_FLAG;
            RI = 0;
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
    // Enable timer 0 interrupts.
    ET1 = 1;
    // Enable serial port interrupts.
    ES = 1;

    // Timers:
    // 16 Bit prescaler on timers 0, 8 bit auto reload on timer 1
    TMOD = 0x21;
    // Enable timer 0
    TR0 = 1;
    // Enable timer 1
    TR1 = 1;

    // Auto reload
    TH1 = DDS_TIMER_RELOAD;
}

void update_timer()
{
}

void update_lcd()
{
#if 1
	char buffer[6] = "00000";
	WriteStringAtPos(0,0,"ADC:");
	sprintf(buffer, "%4.4d", (unsigned short)(g_adc_value*1.221));
	buffer[2] = buffer[1];buffer[3] = buffer[2];
	buffer[4] = 'V'; buffer[1] = '.';
	WriteStringAtPos(0,10,buffer);
	WriteStringAtPos(1,0,"Sinus:");
	sprintf(buffer, "%4.4d", (unsigned short)(g_adc_value*(1.221/5)));
	buffer[3] = 'H'; buffer[4] = 'z';
	WriteStringAtPos(1,10,buffer);
	WriteStringAtPos(1,16,"Hz");
#else
    // TODO: Possible optimization: only update necessary part of LCD.
    char buffer[17];
    sprintf(buffer, "ADC:      %4.4dV", (unsigned short)(g_adc_value * (1.221/1e3)));
    WriteStringAtPos(0, 0, buffer);
    sprintf(buffer, "Frequency: %2.dHz", (unsigned short)(g_adc_value * (1.221/50)));
    WriteStringAtPos(1, 0, buffer);
#endif

}


// ---------------------------------------------------------------------- 
// Serial port stuff:
void init_uart()
{
    if ((PLLCON & 0x07) <= 5)
    {
        //hier mogen we alleen komen als de waarde van de PLL bruikbaar is
        T3FD = 0x2D; //zie databoek t3 als baud rate generator
        T3CON = (((~PLLCON)-2) & 0x07)+0x80; //baudrate instellen afhankelijk van pll
        SCON = 0x50;	//Uart initializeren
    }
}


void answer_uart()
{
    // TODO: Implement 
    static char uart_text[5] = {0};
    static char char_it = 0;

    char input = SBUF;
    
    putchar(input);
    
    // To long.
    if (char_it > 3 || (char_it == 4 && input == '\n'))
    {
		if(input == '\n')
		{
			char_it = 0;
		}
		else
		{
			// Prevent overflow.
			char_it = 4;
		}
		return;
    }
	
	uart_text[char_it] = input; 
	//printf("%s\n\r", uart_text);
	char_it++;
			
    // We only need to fill the buffer.
    if (char_it != 4)
    {
        return;
    }
    char_it = 0;

    // Interpret the command.
    // Hexadecimal freq
    printf("test\n");
    if (strncmp(":FH\r", uart_text, 4) == 0)
    {
        printf("qsgqsdg");
		// TODO: Juiste waarde berekenen
        //printf("Current frequency: %x\n", g_adc_value);
    }
    // Decimal freq
    else if (strncmp(":FD\r", uart_text, 4) == 0)
    {
        // TODO: Juiste waarde berekenen
        printf("Decimal freq: \n");
    }
    // Decimal freq
    else if (strncmp(":VD\r", uart_text, 4) == 0)
    {
        // TODO: Juiste waarde berekenen
        printf("Voltage: \n");
    }
}

void putchar(char c)
{
	TI=0;	
    SBUF=c;
	while(!TI);
}


// ---------------------------------------------------------------------- 
// DDS Stuff:
void init_dac()
{
    DACCON = 0x7D;
}

void dds_interrupt() interrupt 3
{
    static unsigned short count = 0;
    unsigned short it;
    count += g_dds_inc;

    it = count>>4;
    DAC0H = dac_high[it];
    DAC0L = dac_low[it];
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
        g_dds_inc = 1;
    }
    else if (g_dds_inc > 999)
    {
        g_dds_inc = 999;
    }
	
}

