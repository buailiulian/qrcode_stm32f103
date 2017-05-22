#ifndef _led_H
#define _led_H
#include "stm32f10x.h"
#include "public.h"
#define LED GPIO_Pin_All
void LED_Init(void);
void led_display(void);
void led_huayang(void);


#endif 
