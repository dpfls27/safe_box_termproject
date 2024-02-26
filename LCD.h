/*
 * LCD.h
 *
 * Created: 2023-06-05 오후 4:10:57
 *  Author: USER
 */ 

#ifndef __LCD_H__
#define __LCD_H_

#define LCD_WDATA   PORTA
#define LCD_WINST   PORTA
#define LCD_CTRL    PORTG
#define LCD_EN      0
#define LCD_RW      1 
#define LCD_RS      2 

void LCD_Init(void);
void LCD_Data(unsigned char ch);
void LCD_Comm(unsigned char ch);
void LCD_CHAR(unsigned char c);
void LCD_STR(unsigned char *str);
void LCD_Pos(unsigned char row, unsigned char col);
void LCD_Clear(void);
void disp_some(char*, char*, int ms);

#endif