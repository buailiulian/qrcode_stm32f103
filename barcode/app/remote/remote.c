#include "remote.h"
#include "system.h"

/* 保存红外值 */
uint8_t RemoteCode[4];

#define REMOTE_Delay1us(x) SYSTICK_Delay1us(x)
#define REMOTE_Delay1ms(x) SYSTICK_Delay1us(x)

/****************************************************************************
* Function Name  : REMOTE_Init
* Description    : 初始化红外接收的IO和外部中断
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void REMOTE_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 开启GPIO时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /*  配置GPIO的模式和IO口 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;         //选择你要设置的IO口
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  //设置传输速率
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;      //设置输入上拉模式

    /* 初始化GPIO */
    GPIO_Init(GPIOG, &GPIO_InitStructure);	          

/***************************************************************************/
/**************************** 中断设置 *************************************/
/***************************************************************************/

	/* 设置NVIC参数，注意一次只能开启一位，不能几位相或一起打开*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//设置抢占优先级为0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;       //设置从优先级为0；	
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;     //开启的外部中断0通道中断使能
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		     //使能。

    NVIC_Init(&NVIC_InitStructure);

	/* 选择EXTI */
	GPIO_EXTILineConfig(EXTI_GPIO_REMOTE, EXTI_PIN_REMOTE); 

    /* 设置外部中断的模式 */
	EXTI_InitStructure.EXTI_Line = LINE_REMOTE;			    //设置打开外部中断通道
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;			    //打开使能
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;     //中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿触发

    /*初始化EXTI*/
	EXTI_Init(&EXTI_InitStructure);
}

/****************************************************************************
* Function Name  : EXTI15_10_IRQHandler
* Description    : 红外接收的外部中断函数
* Input          : None
* Output         : RemoteCode：读取到的四组红外数据
* Return         : None
****************************************************************************/

void EXTI15_10_IRQHandler(void)
{
    uint8_t num, bit, timeCount;
    uint16_t timeOut;

    if (EXTI_GetITStatus(EXTI_Line15))
	{
        EXTI_ClearITPendingBit(EXTI_Line15); //清除中断标志

        /* 延时7ms再检测，确认是接收到正确的信号 */
        REMOTE_Delay1ms(2);
        if(REMOTE_IN == 0)
        {
            timeOut = 0;
            /* 等待9ms的低电平引导码过去 */
            while(REMOTE_IN == 0)
            {
                REMOTE_Delay1us(100);
                timeOut++;
                if(timeOut > 1000)  //100ms等待超时
                {
                    return;
                }    
            }

            /* 正确等到9ms的低电平引导码过去 */
            if(REMOTE_IN == 1)
            {
                /* 等待4.5ms的高电平引导过去 */
                timeOut = 0;
                while(REMOTE_IN == 1)
                {
                    REMOTE_Delay1us(100);
                    timeOut++;
                    if(timeOut > 1000)   //100ms等待超时
                    {
                        return;
                    }    
                }

                /* 正确等到4.5ms的高电平过去 */
                for(num=0; num<4; num++)      //红外数据共有4组
                {
                    for(bit=0; bit<8; bit++)  //一组数据8位
                    {
                        timeOut = 0;
                        while(REMOTE_IN == 0) //电平信号前面都有560us的低电平做前缀，等待其过去
                        {
                            REMOTE_Delay1us(80);
                            timeOut++;
                            if(timeOut > 200)   //16ms等待超时
                            {
                                return;
                            }        
                        }
                        
                        timeOut = 0;         //测试电平信号的高电平时间长度
                        timeCount = 0;       //清零计时
                        while(REMOTE_IN == 1)
                        {
                            REMOTE_Delay1us(100);
                            timeOut++;
                            timeCount++;
                            if(timeOut > 500)  //50ms等待超时
                            {
                                return;
                            }                            
                        }//end of 测试逻辑电平的高电平

                        RemoteCode[num] >>= 1;
                        if(timeCount >=  10)   //计时超过560us就是逻辑“1”
                        {
                            RemoteCode[num] |= 0x80;        
                        }    
                    }//end of 一组数据的接收    
                }//end of 接收4组数据    
            }//end of 确认9ms引导码过去
        }//end of 确认不是干扰信号          
	}               
}


