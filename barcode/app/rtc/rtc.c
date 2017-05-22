#include "rtc.h"

/* 月份   1  2  3  4  5  6  7  8  9  10 11 12 */
/* 闰年   31 29 31 30 31 30 31 31 30 31 30 31 */
/* 非闰年 31 28 31 30 31 30 31 31 30 31 30 31 */
const uint8_t RtcLeapMonth[12]   = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const uint8_t RtcCommonMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/* 定义一个全局变量保存时钟 */
RTC_TimeTypeDef RTC_Time;

/* 声明内部函数 */
static uint8_t RTC_CheckLeapYear(uint16_t year);
static uint8_t RTC_SetTime(RTC_TimeTypeDef *time);
static void RTC_NVIC_Config(void);
static void RTC_GetTime(void);

/****************************************************************************
* Function Name  : RTC_SetClock
* Description    : 设置时钟
* Input          : *time：要设置的时钟值
* Output         : None
* Return         : None
****************************************************************************/

void RTC_SetClock(RTC_TimeTypeDef *time)
{   
    RTC_EnterConfigMode();                    //允许配置	
    RTC_WaitForLastTask();	                  //等待最近一次对RTC寄存器的写操作完成
    RTC_SetTime(time);                        //设置时间	
    RTC_ExitConfigMode();                     //退出配置模式  
    RTC_GetTime();         //更新时间
}

/****************************************************************************
* Function Name  : RTC_Config
* Description    : 初始化时钟，并初始化内部的时钟信息
* Input          : time：要初始化的时钟
* Output         : None
* Return         : 0：初始化成功；0xFF：初始化失败
****************************************************************************/

int8_t RTC_Config(RTC_TimeTypeDef *time)
{
    uint32_t timeCount;

    if(BKP_ReadBackupRegister(BKP_DR1) != 0x5050)
    {
        /* 使能PWR电源时钟和BKP备份区域外设时钟 */   
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

        PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问 
		BKP_DeInit();	                //复位备份区域 	
		RCC_LSEConfig(RCC_LSE_ON);	    //设置外部低速晶振(LSE),使用外设低速晶振

        /* 检查指定的RCC标志位设置与否，等待低速晶振（LSE）就绪 */
    	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
		{
    		timeCount++;
            if(timeCount > 0x00FFFFF)
            {
                break;
            }
		}

        /* 外部晶振错误，返回设置失败 */
        if(timeCount > 0x00FFFFF)
        {
            return 0xFF;            
        }

        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);   //设置RTC时钟(RTCCLK),选择LSE作为RTC时钟    
		RCC_RTCCLKCmd(ENABLE);	                  //使能RTC时钟
        RTC_WaitForLastTask();	                  //等待最近一次对RTC寄存器的写操作完成
		RTC_WaitForSynchro();		              //等待RTC寄存器同步  
		RTC_ITConfig(RTC_IT_SEC, ENABLE);		  //使能RTC秒中断
		RTC_WaitForLastTask();              	  //等待最近一次对RTC寄存器的写操作完成

		RTC_EnterConfigMode();                    //允许配置	
		RTC_SetPrescaler(32767);                  //设置RTC预分频的值
		RTC_WaitForLastTask();	                  //等待最近一次对RTC寄存器的写操作完成
		RTC_SetTime(time);                        //设置时间	
		RTC_ExitConfigMode();                     //退出配置模式  
		BKP_WriteBackupRegister(BKP_DR1, 0X5050); //向指定的后备寄存器中写入用户程序数据 
    }
    else
    {
        RTC_WaitForSynchro();	           //等待最近一次对RTC寄存器的写操作完成
		RTC_ITConfig(RTC_IT_SEC, ENABLE);  //使能RTC秒中断
		RTC_WaitForLastTask();	           //等待最近一次对RTC寄存器的写操作完成
    }

    RTC_NVIC_Config();     //RCT中断分组设置，开启中断		    				     
	RTC_GetTime();         //更新时间

    return 0;
}

/****************************************************************************
* Function Name  : RTC_IRQHandler
* Description    : RTC时钟的中断函数，用来跟新时间，或者闹钟
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void RTC_IRQHandler(void)
{		 
	if (RTC_GetITStatus(RTC_IT_SEC) != RESET)     //秒钟中断
	{							
		RTC_GetTime();                            //更新时间   
 	}

	if(RTC_GetITStatus(RTC_IT_ALR)!= RESET)       //闹钟中断
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);	      //清闹钟中断	  	   
  	}
     				  								 
	RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW);  //清闹钟中断
	RTC_WaitForLastTask();	  	    			  //等待最近一次对RTC寄存器的写操作完成			 	   	 
}

/****************************************************************************
* Function Name  : RTC_NVIC_Config
* Description    : RTC中断的设置
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

static void RTC_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;		        //RTC全局中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//抢占优先级设置
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	        //响应优先级设置
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		        //使能该通道中断

	NVIC_Init(&NVIC_InitStructure);   //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
}

/****************************************************************************
* Function Name  : RTC_SetTime
* Description    : 设置RTC时钟的计数器初始值
* Input          : time：设置的初始值（注：年份设置从2000到2100年之间）
* Output         : None
* Return         : 0：设置成功；0xFF：设置失败
****************************************************************************/

static uint8_t RTC_SetTime(RTC_TimeTypeDef *time)
{
    uint8_t  leapYear = 0;
    uint16_t i;
    uint32_t secondCount = 0;
    
    /* 确定写入的时间不超过年限 */
    if((time->year < 2000) || (time->year > 2100)) //从2000年到2100年，一共100年
    {
        return 0xFF;             //超过时限返回失败
    }
   
    /* 将所有的年份秒数相加 */
    for(i = RTC_BASE_YEAR; i<time->year; i++)
    {
        if(RTC_CheckLeapYear(i) == 0)          //如果年份是闰年
        {
            secondCount += RTC_LEEP_YEAR_SECOND;     
        }
        else
        {
            secondCount += RTC_COMMON_YEAR_SECOND;    
        }    
    }
    
    
    /* 检测写入年份是闰年还是平年 */
    if(RTC_CheckLeapYear(time->year) == 0) //如果是闰年
    {
        leapYear = 1;                     //标记为闰年            
    }
    else
    {
        leapYear = 0;                     //标记为平年    
    }
    /* 所有月份秒数相加 */
    for(i=1; i<time->month; i++)
    {
        if(leapYear == 1)
        {
            secondCount += RtcLeapMonth[i - 1] * RTC_DAY_SECOND;     
        }
        else
        {
            secondCount += RtcCommonMonth[i - 1] * RTC_DAY_SECOND;
        }       
    }
    
    /* 所有的日期秒数相加 */
    for(i=1; i<time->day; i++)
    {
        secondCount += RTC_DAY_SECOND;
    }
    
    /* 小时的秒数 */
    secondCount += RTC_HOUR_SECOND * time->hour;
    
    /* 分钟的秒数 */
    secondCount += 60 * time->minit;
    
    /* 加上秒数 */
    secondCount += time->second; 

    /* 使能PWR电源时钟和BKP备份区域外设时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	PWR_BackupAccessCmd(ENABLE);	  //使能RTC和后备寄存器访问 
	RTC_SetCounter(secondCount);	  //设置RTC计数器的值 
    
    RTC_WaitForLastTask();	          //等待最近一次对RTC寄存器的写操作完成  	

	return 0;                         //设置成功返回0
}

/****************************************************************************
* Function Name  : RTC_CheckLeapYear
* Description    : 检测年份是否是闰年
* Input          : year：检测的年份
* Output         : None
* Return         : 0：是闰年；0xFF：是平年
****************************************************************************/

static uint8_t RTC_CheckLeapYear(uint16_t year)
{
    /* 闰年有两种计算方式，第一种是能被四整除且不能被100整除， */
    /* 第二种是能被100整除且能被400整除 */

    if((year % 100) == 0) //能被100整除的，且能被400整除是闰年
    {
        if((year % 400) == 0)
        {
            return 0;     //是闰年 
        }
        else
        {
            return 0xFF;  //是平年
        }   
    }
    else                  //不能被100整除，但能被4整除是闰年
    {
        if((year % 4) == 0)
        {
            return 0;     //是闰年
        }
        else
        {
            return 0xFF;  //是平年
        }
    }
}

/****************************************************************************
* Function Name  : RTC_GetTime
* Description    : 读取RTC计数器的值，并将其转化为日期
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/
 
static void RTC_GetTime(void)
{
    uint8_t leapYear = 0, i = 0;
    uint32_t secondCount = 0;
    uint32_t day;

    /* 读取时钟计数器的值 */
    secondCount =  RTC->CNTH;
    secondCount <<= 16;
    secondCount |= RTC->CNTL;
    
    day = secondCount / RTC_DAY_SECOND;           //求出天数
    secondCount = secondCount % RTC_DAY_SECOND;   //求出剩余秒数

    RTC_Time.year = RTC_BASE_YEAR;

    /* 求出星期几 */
    RTC_Time.week = (day + 6) % 7;         //因为2000年1月1日是星期六所以加6

    /* 求出年份 */
    while(day >= 365)
    {
        if(RTC_CheckLeapYear(RTC_Time.year) == 0)   //是闰年
        {
            day -= 366;                    //闰年有366天    
        }
        else
        {
            day -= 365;                    //平年有365天
        }

        RTC_Time.year++;
    }

    /* 求出月份 */
    if(RTC_CheckLeapYear(RTC_Time.year) == 0)
    {
        leapYear = 1;                        //如果是闰年标记
    }

    i = 0;
    RTC_Time.month = 1;
    while(day >= 28)
    {        
        if(leapYear == 1)
        {
            if(day < RtcCommonMonth[i]) //天数不够一个月
            {
                break;
            }
            day -= RtcLeapMonth[i];     //减去闰年该月的天数
        }
        else
        {
            if(day < RtcCommonMonth[i]) //天数不够一个月
            {
                break;
            }
            day -= RtcCommonMonth[i];   //减去平年该月的天数 
        }
        RTC_Time.month++;               //月份加1
        i++;                            //月份数组加1
    }

    /* 求出天数 */
    RTC_Time.day = day + 1;             //月份剩下的天数就是日期(日期从1号开始)

    RTC_Time.hour = secondCount / RTC_HOUR_SECOND;       //求出小时
    RTC_Time.minit = secondCount % RTC_HOUR_SECOND / 60; //求出分钟
    RTC_Time.second = secondCount % RTC_HOUR_SECOND %60; //求出秒
 
}






















