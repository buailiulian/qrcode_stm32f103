#include "usart.h"


void USART1_NVIC_RxConfig(void);
void USART2_NVIC_RxConfig(void);

/****************************************************************************
* Function Name  : USART1_Config
* Description    : Configurates the USART1.
* Input          : baudRate：波特率
* Output         : None
* Return         : None
****************************************************************************/

void USART1_Config(uint16_t baudRate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	/* 打开RCC时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* 设置TXD的GPIO参数 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;               //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;                     //串口输出PA9

	/* 初始化串口输入IO */
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 设置RXD的GPIO参数 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;          //模拟输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                     //串口输入PA10

	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 设置USART的参数 */
	USART_InitStructure.USART_BaudRate = baudRate;                  //波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;     //数据长8位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;          //1位停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;             //无效验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//失能硬件流
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; //开启发送和接受模式
	
    /* 初始化USART1 */
	USART_Init(USART1, &USART_InitStructure);
	
	/* 使能USART1 */
	USART_Cmd(USART1, ENABLE);
#ifdef USE_USART1RX_INTERRUPT  
    USART1_NVIC_RxConfig();
    USART_ITConfig(USART1, USART_IT_RXNE ,ENABLE);
#endif 		
}

/****************************************************************************
* Function Name  : USART2_Config
* Description    : Configurates the USART1.
* Input          : baudRate：波特率
* Output         : None
* Return         : None
****************************************************************************/

void USART2_Config(uint16_t baudRate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	/* 打开RCC时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* 设置TXD的GPIO参数 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;               //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;                     //串口输出PA9

	/* 初始化串口输入IO */
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 设置RXD的GPIO参数 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;          //模拟输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;                      //串口输入PA10

	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 设置USART的参数 */
	USART_InitStructure.USART_BaudRate = baudRate;                  //波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;     //数据长8位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;          //1位停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;             //无效验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//失能硬件流
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; //开启发送和接受模式
	
    /* 初始化USART1 */
	USART_Init(USART2, &USART_InitStructure);
	
	/* 使能USART1 */
	USART_Cmd(USART2, ENABLE);
#ifdef USE_USART2RX_INTERRUPT   
    USART2_NVIC_RxConfig();
    USART_ITConfig(USART2, USART_IT_RXNE ,ENABLE);
#endif 		
}

/****************************************************************************
* Function Name  : USART1_SetWord
* Description    : 通过串口1发送字符串.
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void USART1_SendWord(uint8_t *wd)
{	
	while(*wd)                      //检测是否发送数据是否为空
	{
		USART_SendData(USART1, *wd);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET); //等待发送完毕
		wd++;
	}		
}

/****************************************************************************
* Function Name  : USART2_SetWord
* Description    : 通过串口2发送字符串.
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void USART2_SendWord(uint8_t *wd)
{	
	while(*wd)                      //检测是否发送数据是否为空
	{
		USART_SendData(USART2, *wd);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) != SET); //等待发送完毕
		wd++;
	}		
}

/* 如果要使用printf函数的话要添加以下代码 */
/* 否则就打开Target Options里面的Target位置选择Use MicroLIB */
#ifdef USE_PRINTF

#pragma import(__use_no_semihosting)             
/* 标准库需要的支持函数 */                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef’ d in stdio.h. */ 
FILE __stdout; 
/* 定义_sys_exit()以避免使用半主机模式 */    
_sys_exit(int x) 
{ 
	x = x; 
}

/****************************************************************************
* Function Name  : fputc
* Description    : 使用printf函数要重定向这个fputc函数.
* Input          : ch , *f
* Output         : None
* Return         : ch
****************************************************************************/
 
int fputc(int ch, FILE *f)
{
	
	USART_SendData(USART1, (uint8_t) ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
	
	return ch;	
}
////* 寄存器版 */
//int fputc(int ch, FILE *f)
//{      
//	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
//	USART1->DR = (u8) ch;      
//	return ch;
//}

#endif

#ifdef USE_USART1RX_INTERRUPT

/****************************************************************************
* Function Name  : USART1_NVIC_RxConfig
* Description    : 设置接收中断的中断等级，并打开总中断
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void USART1_NVIC_RxConfig(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 设置NVIC参数 */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级为0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        //响应优先级为0
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;   //打开USART1的全局中断
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		      //使能。

    NVIC_Init(&NVIC_InitStructure);
}

/****************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : 串口1的中断函数
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/
uint8_t USART1_RX_Buff[64], USART1_RX_State;

void USART1_IRQHandler (void)
{
    uint8_t dat;

    /* 接收中断(接收到的数据必须是0x0D 0x0A结尾(即回车))表示接收结束 */
    if(USART_GetITStatus(USART1, USART_IT_RXNE))
    {
        
        dat = USART_ReceiveData(USART1);
        /* 没接收完，使用USART1_RX_State的最高位作结束标识 */
        if(((USART1_RX_State & 0x80) == 0) && ((USART1_RX_State & 0x3F) < 63)) 
        {   
            /* 已经接收到0x0D，使用USART1_RX_State的第6位（从0位开始）作接收到0x0D的标识 */
            if(USART1_RX_State & 0x40)    
            {
                if(dat == 0x0A)    //第二个字节接收到0x0A接收结束
                {
                    USART1_RX_State |= 0x80;
                    USART1_RX_Buff[USART1_RX_State & 0x3F] = 0;   
                }
                else
                {
                    USART1_RX_State = 0; 
                }
            }
            else
            {
                if(dat == 0x0D)     //接收到0x0D，标识USART1_RX_State的第6位(从0位开始)
                {
                    USART1_RX_State |= 0x40;    
                }
                else
                {
                    USART1_RX_Buff[USART1_RX_State & 0x3F] = dat;
                    USART1_RX_State++;    
                }
            } 
        }
    }    
}

#endif

#ifdef USE_USART2RX_INTERRUPT

/****************************************************************************
* Function Name  : USART1_NVIC_RxConfig
* Description    : 设置接收中断的中断等级，并打开总中断
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void USART2_NVIC_RxConfig(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 设置NVIC参数 */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级为0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        //响应优先级为0
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;         //打开USART2的全局中断
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		      //使能。

    NVIC_Init(&NVIC_InitStructure);
}

/****************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : 串口2的中断函数
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/
uint8_t USART2_RX_Buff[64], USART2_RX_State;

void USART2_IRQHandler (void)
{
    uint8_t dat;
    if(USART_GetITStatus(USART2, USART_IT_RXNE))
    {
        dat = USART_ReceiveData(USART2);

        /* 没接收完，使用USART2_RX_State的最高位作结束标识 */
        if(((USART2_RX_State & 0x80) == 0) && ((USART2_RX_State & 0x3F) < 63)) 
        {   
            /* 已经接收到0x0D，使用USART2_RX_State的第6位（从0位开始）作接收到0x0D的标识 */
            if(USART2_RX_State & 0x40)    
            {
                if(dat == 0x0A)    //第二个字节接收到0x0A接收结束
                {
                    USART2_RX_State |= 0x80;
                    USART2_RX_Buff[USART2_RX_State & 0x3F] = 0;   
                }
                else
                {
                    USART2_RX_State = 0; 
                }
            }
            else
            {
                if(dat == 0x0D)     //接收到0x0D，标识USART2_RX_State的第6位(从0位开始)
                {
                    USART2_RX_State |= 0x40;    
                }
                else
                {
                    USART2_RX_Buff[USART2_RX_State & 0x3F] = dat;
                    USART2_RX_State++;    
                }
            } 
        }
    }    
}

#endif














