/***************************************************************************
File Name:  lcd.c
Description: LCD source code file 
             Written for SDCC compiler and P89V51 device
             LCD size is 16 X 2
             LCD controller is JHD 162A

History:
* Ver 1.0 (24th May 2007)
 - First version created

You are free to use or modify this file in your applications. 
We will try our best to ensure the accuracy of content however there is no 
intended or implied guarantee. 
Copyrights 2007 www.embeddedtutorial.com
***************************************************************************/
#include <8051.h>
#include "lcd.h"

#define HIGH	1
#define LOW	0

/* Row starting address */
static unsigned char rowAddress[] = {0,0x40};
/* Internal functions */

/* This is for Four bit mode, modify this for 8 Bit mode */
void WriteLCDDATA (unsigned char x )
{
	P0 = (P0 & 0x0F) | ((x) & 0xF0);	
}
	
	
unsigned char ReadLCDDATA ()
{
	return (P0 & 0xF0);
}		

/*************************************************************************** 
   Name : DeadDelay 
   Description: Insert a specified time delay
   Comments:
   Just for demo. Don't use this function as a general purpose function
   because it is eating up CPU cycle and doing Nothing.
   Prefer some intelligent waiting where some other routine
   can work while you are waiting
***************************************************************************/
static void DeadDelay(unsigned int delay)
{
	unsigned int indexI,indexJ;
	
	for(indexI=0; indexI<100; indexI++)
		for(indexJ=0; indexJ<delay; indexJ++);
}

/*************************************************************************** 
   Name : WriteToLCD 
   Description: Sends data and control signals RS and RW to LCD
   Parameters:
   RS_VAL  - (INPUT)RS signal for LCD
   data    - (INPUT)Data to be written to LCD
   
   Return: None
   Comments:
****************************************************************************/
void WriteToLCD(unsigned char RS_VAL,unsigned char dat)
{  
	RS = RS_VAL;
	
	EN = HIGH;
	WriteLCDDATA(dat);
	DeadDelay(1);			
	EN = LOW;

#ifdef	LCD_MODE_4BIT 
	/* Send lower nibble */
	dat = dat << 4;

	EN = HIGH;
	WriteLCDDATA(dat);
	DeadDelay(1);			
	EN = LOW;
#endif	

	RS = HIGH;

}

/*************************************************************************** 
   Name : DelayWhileLCDBusy 
   Description: Waits while LCD is busy. This function polls Busy bit of LCD
   Parameters:
   timeout  - Maximum Number of time busy flag must be polled before exiting
   Return: None
   Comments:
   This function is used in ClearScreen and GoHome functions where LCD takes 
   long time.
   To avoid blocking in case of hardware failure timeout feature is used which
   will poll busy bit for Maximum of a user defined number times.
****************************************************************************/
void DelayWhileLCDBusy(int timeout)
{
	unsigned char dat;
	
	RS = LOW;
	
	EN = HIGH;	 		

	while(timeout)	
	{
		dat  = ReadLCDDATA();
		if(dat & 0x80)
			break;	
	#ifdef 	LCD_MODE_4BIT 
		dat  = ReadLCDDATA();
	#endif		
		timeout--;
	}

	EN = LOW;	 		
	
	RS = HIGH;
}


/* Global/ Exported functions */

/*************************************************************************** 
   Name : InitializeLCD 
   Description: This function initialize the LCD. Set display lines to 2.
                Makes the cursor blink and place it at home
   Parameters:
   Return: None
   Comments:
****************************************************************************/
void InitializeLCD()
{
	unsigned char dat;

	/*  Wait for supplies to stablize */
	DeadDelay(150);	                                                                                                                                         

	dat = 0x30;
	
	RS = LOW;
	EN = LOW;

	WriteLCDDATA(dat);
	EN = HIGH;
	DeadDelay(1);
	EN = LOW;
	
	DeadDelay(50);
	
	WriteLCDDATA(dat);
	EN = HIGH;
	DeadDelay(1);
	EN = LOW;
	
	DeadDelay(10);
	
	WriteLCDDATA(dat);
	EN = HIGH;
	DeadDelay(1);
	EN = LOW;
	
#ifdef 	LCD_MODE_4BIT 
	dat = 0x20;				/* Data length set 4 Bit ( DL=0 ) */
#else
	dat = 0x30;
#endif		
	WriteLCDDATA(dat);
	EN = HIGH;
	DeadDelay(1);
	EN = LOW;
	
	DeadDelay(50);
				
	dat = 0x28;				/* Display lines = 2 (N=1) and font 5X7 dots (F=0) */
	WriteToLCD(LOW,dat);         
												
	dat = 0x0B;				/*Display off(D=0),cursor on(C=1) and cursor blink(B=1) */
	WriteToLCD(LOW,dat);

	dat = 0x0F;
	WriteToLCD(LOW,dat);		/*Display on(D=1),cursor on(C=1) and make cursor blink(B=1) */

	dat = 0x06;				/* cursor move direction to increment(1/D=1) and not to move the display(S=0) */
	WriteToLCD(LOW,dat);			

	dat = 0x01;				/* Clears all the display and returns the cursor to home */
	WriteToLCD(LOW,dat);
	DeadDelay(50);
}

/*************************************************************************** 
   Name : ClearScreen 
   Description: Clear the screen and put the cursor to home position
   Parameters:
   Return: None
   Comments:
   This function takes approximately 1.6ms to complete. Busy flag is checked 
   in the function.
****************************************************************************/
void ClearScreen()
{
	WriteToLCD(LOW,0x01);   
	DeadDelay(20);
}

/* Cursor functions */

/*************************************************************************** 
   Name : MoveCursorToHome 
   Description: This function moves cursor to home position
   Parameters:
   Return: None
   Comments:
   This function takes approximately 1.6ms to complete. Busy flag is checked 
   in the function.
****************************************************************************/
void MoveCursorToHome()
{
	 WriteToLCD(LOW,0x03);
	 DeadDelay(20);
}

/*************************************************************************** 
   Name : SetCursorPos 
   Description: Moves cursor to desired row and colum position
                Address is calculated by adding row starting address and column
   Parameters:
   row: cursor row , First Row = 0 and so on
   col: cursor column where cursor need to be moved First colum = 0 and so on
   Return: None
   Comments:
****************************************************************************/
void SetCursorPos(unsigned char row, unsigned char col)
{
	unsigned char dat;
	
	if((col<LCD_MAX_COLUMNS)&&(row<LCD_MAX_ROWS))    
	{ 
	   	dat = rowAddress[row] + col + 0x80 ;		/* D7 is set high with 0x80 */

   		WriteToLCD(LOW,dat);
	}
}

/*************************************************************************** 
   Name : SetCursorState 
   Description: Changes cursor state as per user parameters
   Parameters:
   isBlinking:  0 - No Blinking, else blinking
   isOn:	0 - OFF, else On
   Return: None
   Comments:
****************************************************************************/
void SetCursorState(unsigned char isBlinking, unsigned char isOn)
{
	int dat;

	dat=0x0c;  				/* Default display on(D=1) */
 
	if(isOn)
  	{
 	   	dat = dat + 2;	     	        /*Cursor On (C=1) */
    		if(isBlinking)
 	  		dat=dat + 1;    	/*Cursor Blink (B=1) */
 	}	

	WriteToLCD(LOW, dat);   
}


/* Writing functions */
/*************************************************************************** 
   Name : WriteCharAtPos 
   Description: Writes character at user defined row and column 
   Parameters:
   row:  row position, First row = 0 and so on
   col:	 column position, First column = 0 and so on
   ch :  character to be written
   Return: None
   Comments:
****************************************************************************/
void WriteCharAtPos(unsigned char row, unsigned char col, char ch)
{
	/* check for position within visible region */
 	if((col<LCD_MAX_COLUMNS)&&(row<LCD_MAX_ROWS)) 
 	{	
		SetCursorPos(row,col);
		WriteToLCD(HIGH,ch);
	}	
}

/*************************************************************************** 
   Name : WriteStringAtPos 
   Description: Writes string at user defined row and column 
   Parameters:
   row:  row position, First row = 0 and so on
   col:	 column position, First column = 0 and so on
   str :  string to be written
   Return: None
   Comments:
****************************************************************************/
void WriteStringAtPos(unsigned char row, unsigned char col, char *str)
{
	unsigned char index = 0;

	/* check for position within visible region */
 	if((col<LCD_MAX_COLUMNS)&&(row<LCD_MAX_ROWS)) 
 	{
 		SetCursorPos(row,col);
		while(((index + col) < LCD_MAX_COLUMNS) && (str[index]!='\0'))
		{
		   WriteToLCD(HIGH,str[index]);
		   index++;
		}   
	}		   
		   
}

/* Creating custom characters */
/*************************************************************************** 
   Name : CreateCustomChar 
   Description: Creates custom character by writing into CGRAM 
   Parameters:
   charIndex:  Character Index in CGRAM (0..7 are reserved for custom characters)
   charBitmap: Custom character's bitmap. Character bitmap consists of 5X8 pixels.
               Bitmap is stored in 8 bytes. 5 LSBs of each byte store the pixel 
               information.
	       Pixel layout of custom character:
               charBitmap[0]    *     *     *     1    2    3    4    5						
	       charBitmap[1]	*     *	    *     6    7    8    9   10
	       charBitmap[2]	*     *	    *    11   12   13   14   15
               charBitmap[3]	*     *	    *    16   17   18   19   20
               charBitmap[4]	*     *	    *    21   22   23   24   25
               charBitmap[5]	*     *	    *    26   27   28   29   30
               charBitmap[6]	*     *	    *    31   32   33   34   35
	       charBitmap[7]	*     *	    *    36   37   38   39   40
   Return: None
   Comments:
****************************************************************************/
void CreateCustomChar(unsigned char charIndex, unsigned char *charBitmap)
{
	char CGRAMptr;
 	char index;

 	if (!(charIndex < MAX_CUSTOM_CHARS))		/* check for index validity */							
		return;
	
	CGRAMptr= CGRAM_START_ADDR + charIndex * 8;	/* calculate start address based on index */
	 
	for(index=0;index<8;index++)									  		
	{
 		WriteToLCD(LOW,CGRAMptr++);    		
	 	WriteToLCD(HIGH,charBitmap[index]);	  		
 	}
}

