#ifndef __DMA_H
#define __DMA_H


#include "stm32f10x.h"


void DMA_Config(DMA_Channel_TypeDef* DMA_CHx,uint32_t pAddr,uint32_t mAddr,uint16_t length);
void DMA_Enable(DMA_Channel_TypeDef*DMA_CHx, uint32_t length);









#endif

