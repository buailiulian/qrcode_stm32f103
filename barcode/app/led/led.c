#include "led.h"
void LED_Init()	  //端口初始化
{
	GPIO_InitTypeDef GPIO_InitStructure;

	SystemInit();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);

	GPIO_InitStructure.GPIO_Pin=LED;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	GPIO_Write(GPIOC,(u16)(0xff)); 	
}
void led_display()	//LED闪烁
{
	GPIO_SetBits(GPIOC,GPIO_Pin_0);
	delay(6000000);//延时约为1s
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);
	delay(6000000);
}
void led_huayang()	 //led花样跑马灯
{
	u8 i;
	for(i=0;i<8;i++)   //从左到右
	{
		GPIO_Write(GPIOC,(u16)~(0x01<<i));
		delay(2000000);
	}
	for(i=0;i<8;i++)  //从右到左
	{
		GPIO_Write(GPIOC,(u16)~(0x80>>i));
		delay(2000000);
	}
	for(i=0;i<4;i++)  //从右到左
	{
		GPIO_Write(GPIOC,(u16)~(0x11<<i));
		delay(2000000);
	}
	for(i=0;i<4;i++)  //从右到左
	{
		GPIO_Write(GPIOC,(u16)~(0x88>>i));
		delay(2000000);
	}	
}
