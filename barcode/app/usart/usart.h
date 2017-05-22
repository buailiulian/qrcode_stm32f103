#ifndef __USART_H
#define __USART_H


#include "stm32f10x.h"

/* 使用中断接收 */
#define USE_USART1RX_INTERRUPT
#define USE_USART2RX_INTERRUPT

#ifdef USE_USART1RX_INTERRUPT
extern uint8_t USART1_RX_Buff[64], USART1_RX_State;
#endif
#ifdef USE_USART2RX_INTERRUPT
extern uint8_t USART2_RX_Buff[64], USART2_RX_State;
#endif
/* 如果使用printf输出 */
#define USE_PRINTF

#ifdef USE_PRINTF
#include<stdio.h>
int fputc(int ch, FILE *f);
#endif


/* 声明全局函数 */
void USART1_Config(uint16_t baudRate);
void USART1_SendWord(uint8_t *wd);

void USART2_Config(uint16_t baudRate);
void USART2_SendWord(uint8_t *wd);









































#endif
