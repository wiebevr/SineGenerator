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
#define NO_ERROR 0xFF
#define SYNTAX_ERROR 0x1F
#define TO_LONG_ERROR 0x3F
static unsigned volatile char g_uart_error_status = NO_ERROR;

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
void error_code_interrupt() interrupt 10;

// Helper functions
void process_input_string(char *input);


void main(void)
{
    /* Set the clock rate to 16 MHz */
    PLLCON = 0x00;

	init_adc();
    init_dac();
    init_interrupts();
    init_uart();
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
            //P2 = g_uart_error_status;
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
    // Enable timer 0 interrupts.
    ET0 = 1;
    // Enable timer 0 interrupts.
    ET1 = 1;
    // Disable serial port interrupts!
    ES = 0;

    // Timers:
    // 16 Bit prescaler on timers 0, 8 bit auto reload on timer 1
    TMOD = 0x21;
    // Enable timer 0
    TR0 = 1;
    // Enable timer 1
    TR1 = 1;

    // Auto reload
    TH1 = DDS_TIMER_RELOAD;


    // Configure TIC
    //ETI = 1;
    IEIP2 = 0x04;
    TIMECON = 3;
    INTVAL = 64;

}

void update_timer()
{
}

void update_lcd()
{
#if 0
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
    char freq[10];
    char voltage[10];
    sprintf(freq, " %03.3d Hz", (unsigned short)(g_adc_value*(1.221/5)));
    // Add point
    freq[0] = freq[1];
    freq[1] = freq[2];
    freq[2] = '.';

    sprintf(voltage, " %03.3d V", (unsigned short)(g_adc_value * 0.1221));
    voltage[0] = voltage[1];
    voltage[1] = '.';

    WriteStringAtPos(0, 0, freq);
    WriteStringAtPos(0, 10, voltage);
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
    static char input_string[7] = {0};
    static char input_it = 0;

    char input = SBUF;
    
    putchar(input);

    // Clear error status when ':' is received.
    if (input == ':')
    {
        g_uart_error_status = NO_ERROR;
    }
    
    if (input == '\r')
    {
        // New line
        input_string[input_it] = '\0';
        process_input_string(input_string);
        input_it = 0;
    }
    else if (input_it > 3)
    {
        // Input to long
        input_it = 4;
        g_uart_error_status = TO_LONG_ERROR;
    }
    else
    {
        input_string[input_it] = input;
        input_it++;
    }
}
void process_input_string(char *input)
{
    // Little hack we could not allocate enough memory on the stack so we 
    // reuse the input buffer.
    char *buffer = input;

    printf("\n");
    if (strcmp(input, ":FH") == 0)
    {
        printf("%x\n\r", (unsigned int)(g_adc_value*(1.221/5)));

    }
    else if (strcmp(input, ":FD") == 0)
    {
        sprintf(buffer, " %03.3d Hz", (unsigned short)(g_adc_value*(1.221/5)));
        // Add point
        buffer[0] = buffer[1];
        buffer[1] = buffer[2];
        buffer[2] = '.';
        printf("%s\n\r", buffer);
    }
    else if (strcmp(input, ":VD") == 0)
    {
        sprintf(buffer, " %03.3d V", (unsigned short)(g_adc_value * 0.1221));
        buffer[0] = buffer[1];
        buffer[1] = '.';
        printf("%s\n\r", buffer);
    }
    else
    {
        g_uart_error_status = SYNTAX_ERROR;
    }
}

void putchar(char c)
{
    TI=0;	
    SBUF=c;
    while(!TI);
}

// ---------------------------------------------------------------------- 
// Error codes flickering:
void error_code_interrupt() interrupt 10
{
    HTHSEC = 0;
    if (P2 == 0xFF)
    {
        P2 = g_uart_error_status;
    }
    else
    {
        P2 = 0xFF;
    }
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

