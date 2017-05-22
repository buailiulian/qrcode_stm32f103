#ifndef __REMOTE_H
#define __REMOTE_H

#include "stm32f10x.h"
#include "system.h"

extern uint8_t RemoteCode[4];

/* IO¿Ú¶¨Òå */
#define EXTI_GPIO_REMOTE   (GPIO_PortSourceGPIOG)
#define EXTI_PIN_REMOTE    (GPIO_PinSource15)
#define LINE_REMOTE        (EXTI_Line15)
#define REMOTE_IN          PGin(15)


void REMOTE_Init(void);




#endif
