#include "lpwr.h"

/****************************************************************************
* Function Name  : LPWR_Config
* Description    : 初始化PA0的外部中断，以用来将单片机从待机模式唤醒
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void LPWR_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

    /* 开启GPIO时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	  //PA.0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //上拉输入

	GPIO_Init(GPIOA, &GPIO_InitStructure);	      //初始化IO

/***************************************************************************/
/**************************** 中断设置 *************************************/
/***************************************************************************/

	/* 设置NVIC参数，注意一次只能开启一位，不能几位相或一起打开*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//设置抢占优先级为0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;       //设置从优先级为0；	
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;         //开启的外部中断0通道中断使能
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		     //使能。

    NVIC_Init(&NVIC_InitStructure);

	/* 选择EXTI */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0); //选择PA0做外部中断

    /* 设置外部中断的模式 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;			    //设置打开外部中断通道
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;			    //打开使能
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;     //中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //上升沿触发

    /*初始化EXTI*/
	EXTI_Init(&EXTI_InitStructure);

}

/****************************************************************************
* Function Name  : LPWR_EnterStandbyMode
* Description    : 进入待机模式
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void LPWR_EnterStandbyMode(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);	//使能PWR外设时钟
	PWR_WakeUpPinCmd(ENABLE);                           //使能唤醒管脚功能
	PWR_EnterSTANDBYMode();	                            //进入待命（STANDBY）模式 		    
}

/****************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void EXTI0_IRQHandler(void)
{ 		    		    				     		    
	EXTI_ClearITPendingBit(EXTI_Line0);                 //清除LINE10上的中断标志位		  
}









