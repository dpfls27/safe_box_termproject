/*
 * LCD.h
 *
 * Created: 2023-06-05 오후 4:10:57
 *  Author: USER
 */ 

#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// 음계
#define   DO   1908 //1908us
#define   RE   1700 //1701us
#define   MI   1515 //1515us
#define   FA   1432 //1433us
#define   SOL  1275 //1351us
#define   LA   1136 //1136us
#define   SI   1012 //1012us
#define   DO5   956 // 956us
#define   RE5   852 // 852us
#define   MI5   759// 1517us

// 음계 번호
#define DO_NUM 1
#define RE_NUM 2
#define MI_NUM 3
#define FA_NUM 5
#define SOL_NUM 6
#define LA_NUM 7
#define SI_NUM 9
#define DO5_NUM 10
#define RE5_NUM 11
#define STAR 13
#define ZERO 14
#define SHARP 15


// 소리 지속 시간
#define HALF_SEC 50000
#define SAFE_SEC 100000
#define ERR_SEC 30000
#define DELAY_SEC 5000

// 동작 모드
// 초기모드, 비밀번호 입력 모드, 인터럽트모드, 도어오픈 모드, 마스터 모드
#define init_mode 0
#define password_input_mode 1
#define interrupt_mode 2
#define door_open_mode 3
#define dangerous_mode 4
#define safe_mode 5 //master mode 의미
#define vibration_sense_mode 6 

#define PASSWORD_SIZE 14

#define ERROR_COUNT 3

#endif