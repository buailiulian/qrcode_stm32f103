#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

//#define USE_EXTI    //使用外部中断

/* 定义要设置的按键使用的PIN口 */
#define GPIO_Pin_KEY (GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4)

/* 按键读取 */
#define KEY0 GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)
#define KEY1 GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2)
#define KEY2 GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)
#define KEY3 GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4)

/* 定义按键的键值 */
#define KEY_UP    0x01
#define KEY_DOWN  0x02
#define KEY_LEFT  0x04
#define KEY_RIGHT 0x08
#define KEY_ALL   0x0F
/* 声明全局函数 */
void KEY_Config(void);
uint8_t KEY_Scan(void);
void KEY_NVIC_Config(uint8_t key);





#endif
