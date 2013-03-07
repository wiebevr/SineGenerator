/***************************************************************************
File Name:  lcd.h
Description: LCD fucntions header file
             This file also contains portability information. Data Pins can be changed
             over here if interfaced differently than stated in example
             LCD Connection
             RS Pin - Port 0 Bit 0
             EN Pin - Port 0 Bit 2
             Data   - Port 0 Bit 4-7

History:
* Ver 1.0 (24th May 2007)
 - First version created

You are free to use or modify this file in your applications. 
We will try our best to ensure the accuracy of content however there is no 
intended or implied guarantee. 
Copyrights 2007 www.embeddedtutorial.com
***************************************************************************/

#ifndef __LCD_H__
#define __LCD_H__

/* Data is send in 4 Bit mode */
#define LCD_MODE_4BIT		1

#define LCD_MAX_ROWS 		2
#define LCD_MAX_COLUMNS 	16
#define MAX_CUSTOM_CHARS 	8
#define CGRAM_START_ADDR 	0x40

/* LCD Signals/ Pins */
#define RS P0_3
#define EN P0_2

/* Function headers */

/* utilities */
void InitializeLCD();
void ClearScreen();
void MoveCursorToHome();

/* Cursor functions */
void SetCursorPos(unsigned char row, unsigned char col);
void SetCursorState(unsigned char isBlinking, unsigned char isOn);
#define SetCursorBlink()    SetCursorState(1,1)
#define SetCursorNoBlink()  SetCursorState(0,1)
#define SetCursorOn()       SetCursorState(0,1)
#define SetCursorOff()      SetCursorState(0,0)

/* Writing functions */
void WriteCharAtPos(unsigned char row, unsigned char col, char ch);
void WriteStringAtPos(unsigned char row, unsigned char col, char *str);

/* Creating custom characters */
void CreateCustomChar(unsigned char charIndex, unsigned char *charBitmap);

#endif /* __LCD_H__ */
