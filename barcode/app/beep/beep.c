#include "beep.h"
#include "system.h"

/****************************************************************************
* Function Name  : BEEP_Config
* Description    : 初始化蜂鸣器的IO配置
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void BEEP_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;               //声明一个结构体变量，用来初始化GPIO

    /* 开启GPIO时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /*  配置GPIO的模式和IO口 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;          //选择你要设置的IO口
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  //设置传输速率
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   //设置推挽输出模式

    /* 初始化GPIO */
    GPIO_Init(GPIOB, &GPIO_InitStructure);	
                       
}

/****************************************************************************
* Function Name  : BEEP_Work
* Description    : 蜂鸣器响滴一声
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void BEEP_Work(void)
{
    uint16_t i;
    uint8_t j;

    for(j=0; j<50; j++)           //产生一段时间的PWM波，使蜂鸣器发出滴一声
    {
        PBout(5) = 1;             //通过我们定义的位操作宏操作PB5口输出高电平
        for(i=0; i<4500; i++);    //延时调整PWM的频率
        PBout(5) = 0; 
        for(i=0; i<4500; i++);   
    }
}

