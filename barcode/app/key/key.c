#include "key.h"
#include "usart.h"

/* 声明函数 */
static void KEY_Delay10ms(void);

/****************************************************************************
* Function Name  : KEY_Config
* Description    : 初始化按键是用的IO口
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void KEY_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 开启GPIO时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    /*  配置GPIO的模式和IO口 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_KEY;        //选择你要设置的IO口
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  //设置传输速率
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;      //设置输入上拉模式

    /* 初始化GPIO */
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    	          
    /* PA0 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;          //选择你要设置的IO口
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;      //设置输入上拉模式

    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

#ifdef USE_EXTI
/****************************************************************************
* Function Name  : KEY_NVIC_Config
* Description    : 初始化外部中断
* Input          : key：选择使用外部中断的按键
* Output         : None
* Return         : None
****************************************************************************/

void KEY_NVIC_Config(uint8_t key)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 打开时钟使能 */
    if(key & KEY_UP)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    }
    if(key & (KEY_DOWN | KEY_LEFT | KEY_RIGHT))
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);    
    }
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* 设置GPIO参数 */    
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  //设置传输速率
    if(key & KEY_UP)
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;      //设置输入下拉模式
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;          //选择你要设置的IO口
    	/*初始化GPIO*/
    	GPIO_Init(GPIOA, &GPIO_InitStructure);
    }
    if(key & (KEY_DOWN | KEY_LEFT | KEY_RIGHT))
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;      //设置输入上拉模式
        if(key & KEY_LEFT)
        {
            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;          //选择你要设置的IO口
        }
        if(key & KEY_DOWN)
        {
            GPIO_InitStructure.GPIO_Pin |= GPIO_Pin_3;          //选择你要设置的IO口
        }
        if(key & KEY_RIGHT)
        {
            GPIO_InitStructure.GPIO_Pin |= GPIO_Pin_4;          //选择你要设置的IO口
        }
    	/*初始化GPIO*/
    	GPIO_Init(GPIOA, &GPIO_InitStructure);        
    }

/***************************************************************************/
/**************************** 中断设置 *************************************/
/***************************************************************************/

	/* 设置NVIC参数，注意一次只能开启一位，不能几位相或一起打开*/
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//设置抢占优先级为0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;       //设置从优先级为0；	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		     //使能。
    
    if(key & KEY_UP)
    {
        NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;         //开启的外部中断0通道中断使能
        NVIC_Init(&NVIC_InitStructure);
    }
    if(key & KEY_LEFT)
    {
        NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;         //开启的外部中断0通道中断使能
        NVIC_Init(&NVIC_InitStructure); 
    }
    if(key & KEY_DOWN)
    {
        NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;         //开启的外部中断0通道中断使能
        NVIC_Init(&NVIC_InitStructure); 
    }
    if(key & KEY_RIGHT)
    {
        NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;         //开启的外部中断0通道中断使能
        NVIC_Init(&NVIC_InitStructure); 
    }

	/* 选择EXTI */
    if(key & KEY_UP)
    {
	    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0); //选择KEY_UP做外部中断
    }
    if(key & KEY_LEFT)
    {
	    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource2); //选择KEY_UP做外部中断
    }
    if(key & KEY_DOWN)
    {
	    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource3); //选择KEY_UP做外部中断
    }
    if(key & KEY_RIGHT)
    {
	    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource4); //选择KEY_UP做外部中断
    }

    /* 设置外部中断的模式 */    
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;			    //打开使能
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;     //中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //上升沿触发

    if(key & KEY_UP)
    {
	    EXTI_InitStructure.EXTI_Line = EXTI_Line0;			    //设置打开外部中断通道
        /*初始化EXTI*/
	    EXTI_Init(&EXTI_InitStructure);
	}
    if(key & (KEY_DOWN | KEY_LEFT | KEY_RIGHT))
    {
        if(key & KEY_LEFT)
        {
            EXTI_InitStructure.EXTI_Line |= EXTI_Line2;
        }
        if(key & KEY_DOWN)
        {
            EXTI_InitStructure.EXTI_Line |= EXTI_Line3;
        }
        if(key & KEY_RIGHT)
        {
            EXTI_InitStructure.EXTI_Line |= EXTI_Line4;
        }
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  //下降沿触发
        /*初始化EXTI*/
    	EXTI_Init(&EXTI_InitStructure);
    }
}

/****************************************************************************
* Function Name  : EXTI0_IRQHandler
* Description    : 外部中断0的中断函数
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void EXTI0_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line0))
	{
        printf(" KEY_UP发生中断！\n");
	}
	EXTI_ClearITPendingBit(EXTI_Line0);
}


/****************************************************************************
* Function Name  : EXTI2_IRQHandler
* Description    : 外部中断2的中断函数
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void EXTI2_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line2))
	{
        printf(" KEY_LEFT发生中断！\n");
	}
	EXTI_ClearITPendingBit(EXTI_Line2);
}

/****************************************************************************
* Function Name  : EXTI3_IRQHandler
* Description    : 外部中断3的中断函数
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void EXTI3_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line3))
	{
        printf(" KEY_DOWN发生中断！\n");
	}
	EXTI_ClearITPendingBit(EXTI_Line3);
}

/****************************************************************************
* Function Name  : EXTI4_IRQHandler
* Description    : 外部中断4的中断函数
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void EXTI4_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line4))
	{
        printf(" KEY_RIGHT发生中断！\n");
	}
	EXTI_ClearITPendingBit(EXTI_Line4);
}

#endif

/****************************************************************************
* Function Name  : KEY_Delay10ms
* Description    : 按键使用的消抖延时函数。注意：该延时函数具体延时时间是不确
*                * 定的，并非真的就延时10ms，要精确延时请使用定时器。
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

static void KEY_Delay10ms(void)
{
    uint16_t i;
    for(i=0; i<0x5FFF; i++);
}

/****************************************************************************
* Function Name  : KEY_Scan
* Description    : 按键扫描，注意该按键扫描支持一次按1个键，不支持同时按多个键
* Input          : None
* Output         : None
* Return         : keyValue：按键的键值
****************************************************************************/

uint8_t KEY_Scan(void)
{
    uint8_t keyValue = 0, timeCount = 0;

    if((KEY0 == 1) || (KEY1 == 0) || (KEY2 == 0) || (KEY3 == 0)) //检测是否有按键按下
    {
        KEY_Delay10ms();                                      //延时消抖

        /* 检测是哪个按键按下 */
        if(KEY0 == 1)
        {
            keyValue = KEY_UP;
        }
        else if(KEY1 == 0)
        {
            keyValue = KEY_LEFT;
        }
        else if(KEY2 == 0)
        {
            keyValue = KEY_DOWN;
        }
        else if(KEY3 == 0)
        {
            keyValue = KEY_RIGHT;
        }
        else
        {
            keyValue = 0;
        }
        
        /* 有按键按下时，做松手检测 */
        if(keyValue != 0)
        {
            while(((KEY0 == 1) || (KEY1 == 0) || (KEY2 == 0) || (KEY3 == 0)) && (timeCount < 150))
            {
                KEY_Delay10ms();
                timeCount++;        
            }
//            KEY_Delay10ms();//由于主函数中程序较少，连续扫描的速度太快，加一个松手消抖减少误读   
        }        
    }

    return keyValue;
}












