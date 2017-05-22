#ifndef __IIC_H
#define __IIC_H

#include "stm32f10x.h"

/* 定时使用的IO口 */
#define IIC_SDA_IN()  {GPIOB->CRH &= 0XFFFF0FFF;GPIOB->CRH |= 8 << 12;\
GPIOB->BSRR = 1 << 11;} //输入(上拉)
#define IIC_SDA_OUT() {GPIOB->CRH &= 0XFFFF0FFF;GPIOB->CRH |= 3 << 12;} //推挽输出

#define IIC_SDA_SET {GPIOB->BSRR = 1 << 11;}
#define IIC_SDA_CLR {GPIOB->BRR = 1 << 11;}
#define IIC_SDA     (GPIOB->IDR & (1 << 11))
#define IIC_SCL_SET {GPIOB->BSRR = 1 << 10;}
#define IIC_SCL_CLR {GPIOB->BRR = 1 << 10;}

/* 声明全局函数 */
void IIC_Config(void);
void IIC_Start(void);
void IIC_Stop(void);
void IIC_SendData(uint8_t dat);
uint8_t IIC_ReceiveData(uint8_t ack);
int8_t IIC_WaitAck(void);




























#endif
