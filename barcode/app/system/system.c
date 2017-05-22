#include "system.h"

#ifdef USE_UCOSII_OS
#include "os_includes.h"
#endif

/****************************************************************************
* Function Name  : SYSTEM_SetClock
* Description    : 设置外部时钟作为PLL输入，并设置相应的时钟频率。注意时钟频率
*                * 要是8的倍数。该函数只提供32MHZ到72MHZ的设置。
* Input          : freq：频率参数（该参数只能设置为8的倍数，从4倍到9倍）
* Output         : None
* Return         : -1：表示设置失败，时钟返回初始化状态
*                * 0：表示初始化成功
****************************************************************************/

int8_t SYSTEM_SetClock(uint8_t freq)
{
    ErrorStatus HSEStartUpStatus;
    uint32_t pllMul;

    /* 选择倍频系数 */
    switch(freq)
    {
        case(32):
            pllMul = RCC_PLLMul_4;
            break;
        case(40):
            pllMul = RCC_PLLMul_5;
            break;
        case(48):
            pllMul = RCC_PLLMul_6;
            break;
        case(56):
            pllMul = RCC_PLLMul_7;
            break;
        case(64):
            pllMul = RCC_PLLMul_8;
            break;
        case(72):
            pllMul = RCC_PLLMul_9;
            break;
        default:
            RCC_DeInit();
            return -1;
    }
        
    /* 复位RCC */
    RCC_DeInit();
    
    /* 使能打开HSE外部高速时钟 */
    RCC_HSEConfig(RCC_HSE_ON);

    /* 等待HSE时钟就绪 */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
    
    /* 当HSE时钟就绪 */
    if (HSEStartUpStatus == SUCCESS)
    {
        /* 使能开启FLASH ,允许程序去操作FLASH上面的数据，要是不设置程序会出错 */
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
        
        /* FLASH代码2个延时周期 */
        FLASH_SetLatency(FLASH_Latency_2); 
       
        /* 设置AHB总线的时钟分频 */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);        //不分频
        
        /* 设置高速时钟的时钟分频（最大72MHZ） */
        RCC_PCLK2Config(RCC_HCLK_Div1);         //不分频
        
        /* 设置低速时钟的时钟分频（最大36MHZ） */
        RCC_PCLK1Config(RCC_HCLK_Div2);         //二分频
        
        /* 设置PLL的时钟输入和倍频数 */
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, pllMul);//选择HSE不分频输入，倍频pllMul倍
        
        /* 开启 PLL */ 
        RCC_PLLCmd(ENABLE);
        
        /* 等待PLL时钟就绪 */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }

        /* 选择PLL时钟作为输入 */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* 等待系统时钟切换完成 */
        while(RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }
    else              //若HSE时钟开始失败，返回设置失败
    {
        return -1;  
    }

    return 0;         //返回设置成功
}

/****************************************************************************
* Function Name  : NVIC_Config
* Description    : 设置NVIC中断系统抢占优先和从优先的配置（注:现设置为3位抢占
*                * 优先，1位从优先。）
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void NVIC_Config(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);    
}

#ifndef USE_UCOSII_OS
/****************************************************************************
* Function Name  : SYSTICK_Init
* Description    : 开始系统滴答定时器，并设置定时时间。
*                * 它的中断函数在stm32f10x_it.c的139行。
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void SYSTICK_Config(void)
{
    /* 时钟是72MHZ时，所以计数72次(最大为16777215)，就是1us */
	/* 主要是为了设置时钟 */
	if(SysTick_Config(72) == 0)//开启成功返回0
	{
		/* 调用这个设置函数的时候默认是直接打开计数器的，现在把它关掉。 */
		/* 关闭中断 */	
		 SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);		 	
	}    
}

/****************************************************************************
* Function Name  : SYSTICK_Delay1us
* Description    : 延时函数。
* Input          : us：延时的时间
* Output         : None
* Return         : None
****************************************************************************/

void SYSTICK_Delay1us(uint16_t us)
{
	uint32_t countValue;

	SysTick->LOAD  = (us * 72) - 1;             //设置重装数值, 72MHZ时
	SysTick->VAL   = 0;	                        //清零计数器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;	//打开计时器

	do
	{
		countValue = SysTick->CTRL; 	
	}
	while(!(countValue & (1 << 16)));			//等待时间到来

	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;	//关闭计数器	
}

/****************************************************************************
* Function Name  : SYSTICK_Delay1ms
* Description    : 延时函数。
* Input          : ms：延时的时间
* Output         : None
* Return         : None
****************************************************************************/

void SYSTICK_Delay1ms(uint16_t ms)
{
	uint32_t countValue;

	SysTick->LOAD  = 72000 - 1;                 //设置重装数值, 72MHZ时延时1ms
	SysTick->VAL   = 0;	                        //清零计数器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;	//打开计时器

	do
	{
		countValue = SysTick->CTRL;
		if(countValue & (1 << 16))              //当到1ms是计数减1
		{
			ms--;
		} 	
	}
	while(ms);			                        //等待时间到来

	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;	//关闭计数器	
}

/****************************************************************************
* Function Name  : SYSTICK_Delay1s
* Description    : 延时函数。
* Input          : s：延时的时间
* Output         : None
* Return         : None
****************************************************************************/

void SYSTICK_Delay1s(uint16_t s)
{
	uint32_t countValue;

	s *= 5; 
	SysTick->LOAD  = 72000 * 200 - 1;           //设置重装数值, 72MHZ时延时200ms
	SysTick->VAL   = 0;	                        //清零计数器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;	//打开计时器

	do
	{
		countValue = SysTick->CTRL;
		if(countValue & (1 << 16))              //当到200ms是计数减1
		{
			s--;
		} 	
	}
	while(s);			                        //等待时间到来

	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;	//关闭计数器	
}

#else


/****************************************************************************
* Function Name  : SYSTICK_Delay1us
* Description    : 延时函数。
* Input          : us：延时的时间
* Output         : None
* Return         : None
****************************************************************************/

void SYSTICK_Delay1us(uint16_t us)
{
    uint32_t oldValue, newValue, ticks, reloadValue, countValue = 0;

    /* 读取重装值 */
    reloadValue = SysTick->LOAD;
    
    /* 计算延时需要多少个滴答，现在设置的延时是在时钟为72MHZ的基础上设置的 */
    ticks = us * 72;

    oldValue = SysTick->VAL;  //刚进入时的计数器值
    while(1)
    {
        newValue = SysTick->VAL;
        if(newValue != oldValue)
        {
            if(newValue < oldValue)    //滴答计数器是倒数计数器
            {
                countValue += oldValue - newValue;
            }
            else
            {
                countValue += reloadValue - newValue + oldValue;
            }
            if(countValue >= ticks)
            {
                break;
            }    
        }
    }   	
}

/****************************************************************************
* Function Name  : SYSTICK_Delay1ms
* Description    : 延时函数。
* Input          : ms：延时的时间
* Output         : None
* Return         : None
****************************************************************************/

void SYSTICK_Delay1ms(uint16_t ms)
{
    if(OSRunning != 0)
    {
        OSTimeDlyHMSM(0, 0, 0, ms); 
    }
    else
    {
        while(ms--)
        {
            SYSTICK_Delay1us(1000);    
        }    
    }   	
}

/****************************************************************************
* Function Name  : SYSTICK_Delay1s
* Description    : 延时函数。
* Input          : s：延时的时间
* Output         : None
* Return         : None
****************************************************************************/

void SYSTICK_Delay1s(uint16_t s)
{
    OSTimeDlyHMSM(0, 0, s, 0);    	
}

#endif

