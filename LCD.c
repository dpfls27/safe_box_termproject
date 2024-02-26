/*
 * CFile1.c
 *
 * Created: 2023-06-11 오전 1:28:16
 *  Author: USER
 */ 

#include "LCD.h"
#include "COMPONENT.h"

// LCD 관련한 함수들 다 여기 

void LCD_Init(void){
	LCD_Comm(0x30);
	_delay_us(4100);
	LCD_Comm(0x30);
	_delay_us(100);
	LCD_Comm(0x30);
	_delay_us(100);
	LCD_Comm(0x38);
	_delay_us(1000);
	//LCD_Comm(0x0e); // 맨뒤 짝대기
	LCD_Comm(0x0c); // 맨뒤 없어짐
	//LCD_Comm(0x0f); //맨뒤에 깜빡거림
	_delay_us(1000);
	LCD_Comm(0x01);
	_delay_us(2000);
	LCD_Comm(0x06);
	_delay_us(1000);
}

void LCD_Data(unsigned char ch){
	LCD_CTRL |= (1<<LCD_RS);
	LCD_CTRL &= ~(1<<LCD_RW);
	LCD_CTRL |= (1<<LCD_EN);
	_delay_ms(50);
	LCD_WDATA = ch;
	_delay_ms(50);
	LCD_CTRL &= ~(1<<LCD_EN);
}

void LCD_Comm(unsigned char ch){
	LCD_CTRL &= ~(1<<LCD_RS);
	LCD_CTRL &= ~(1<<LCD_RW);
	LCD_CTRL |= (1<<LCD_EN);
	_delay_ms(50);
	LCD_WINST = ch;
	LCD_CTRL &= ~(1<<LCD_EN);
}

void LCD_CHAR(unsigned char c){
	LCD_Data(c);
	_delay_ms(1);
}

void LCD_STR(unsigned char *str){
	while(*str != 0){
		LCD_CHAR(*str);
		str++;
	}
}

void LCD_Pos(unsigned char row, unsigned char col){
	LCD_Comm( 0x80 | ((row*0x40)+col));
}

void LCD_Clear(void){
	LCD_Comm(0x01);
	_delay_ms(2);
}


void LCD_work(unsigned char *str, unsigned char *str1){
	LCD_Clear();
	LCD_Pos(0,0);   //100us
	LCD_STR(str);  //100us
	LCD_Pos(1,0);   //100us
	LCD_STR(str1); //100us
}