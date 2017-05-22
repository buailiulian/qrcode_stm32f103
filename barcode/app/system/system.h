#ifndef __SYSTEM_H
#define __SYSTEM_H

#include"stm32f10x.h"

//#define USE_UCOSII_OS    //使用UCOSII的时候定义这个
/* 定义将位段区地址转换为位带别名区地址的宏 */
#define BIT_BAND(addr, bitNum) ((addr & 0xF0000000)+ 0x2000000\
+ ((addr &0xFFFFF) << 5) + (bitNum<<2))

/* 将数值转换位指针地址 */ 
#define MEM_Addr(bitAddr) (*(volatile uint32_t *)bitAddr) 

/* GPIO的输入输出寄存器地址映射 */
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8)  //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8)  //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8)  //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8)  //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8)  //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8)  //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8)  //0x40011E08 

/* 定义输出位操作的宏 */
#define PAout(bitNum) MEM_Addr(BIT_BAND(GPIOA_ODR_Addr, bitNum))  
#define PBout(bitNum) MEM_Addr(BIT_BAND(GPIOB_ODR_Addr, bitNum))  
#define PCout(bitNum) MEM_Addr(BIT_BAND(GPIOC_ODR_Addr, bitNum))  
#define PDout(bitNum) MEM_Addr(BIT_BAND(GPIOD_ODR_Addr, bitNum))  
#define PEout(bitNum) MEM_Addr(BIT_BAND(GPIOE_ODR_Addr, bitNum))  
#define PFout(bitNum) MEM_Addr(BIT_BAND(GPIOF_ODR_Addr, bitNum))  
#define PGout(bitNum) MEM_Addr(BIT_BAND(GPIOG_ODR_Addr, bitNum))  

/* 定义输入位操作的宏 */
#define PAin(bitNum)  MEM_Addr(BIT_BAND(GPIOA_IDR_Addr, bitNum))
#define PBin(bitNum)  MEM_Addr(BIT_BAND(GPIOB_IDR_Addr, bitNum))
#define PCin(bitNum)  MEM_Addr(BIT_BAND(GPIOC_IDR_Addr, bitNum))
#define PDin(bitNum)  MEM_Addr(BIT_BAND(GPIOD_IDR_Addr, bitNum))
#define PEin(bitNum)  MEM_Addr(BIT_BAND(GPIOE_IDR_Addr, bitNum))
#define PFin(bitNum)  MEM_Addr(BIT_BAND(GPIOF_IDR_Addr, bitNum))
#define PGin(bitNum)  MEM_Addr(BIT_BAND(GPIOG_IDR_Addr, bitNum))


/* 声明全局函数 */
int8_t SYSTEM_SetClock(uint8_t freq);
void NVIC_Config(void);
void SYSTICK_Config(void);
void SYSTICK_Delay1us(uint16_t us);
void SYSTICK_Delay1ms(uint16_t ms);
void SYSTICK_Delay1s(uint16_t s);







#endif
