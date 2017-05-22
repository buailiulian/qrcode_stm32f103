#ifndef __CAN_H
#define __CAN_H

#include "stm32f10x.h"

/* 如果要使用中断接收数据的话，定义 */
#define CAN_RX0_INT_ENABLE

#ifdef CAN_RX0_INT_ENABLE
extern uint8_t CAN_RX_BUFF[8];
#endif


void CAN1_Config(uint8_t mode);
void CAN1_SendMesg(uint32_t id, uint8_t len, uint8_t *dat);
void CAN1_Config16BitFilter(uint16_t id1, uint16_t id2);
void CAN1_Config32BitFilter(uint32_t id);
void CAN1_ReceiveMesg(uint8_t *receiveBuff);






















#endif
