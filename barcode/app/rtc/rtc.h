#ifndef __RTC_H
#define __RTC_H

#include "stm32f10x.h"

/* 定义时钟的结构体 */
typedef struct{
uint8_t  second;
uint8_t  minit;
uint8_t  hour;
uint8_t  week;
uint8_t  day;
uint8_t  month;
uint16_t year;
}RTC_TimeTypeDef;

/* 保存时钟信息 */
extern RTC_TimeTypeDef RTC_Time;

/* 定义时钟设置 */
#define RTC_BASE_YEAR      (uint16_t)2000                  //年份最小设置为2000年

#define RTC_LEEP_YEAR_SECOND    (uint32_t)(366 * 24 * 60 * 60)  //闰年秒数
#define RTC_COMMON_YEAR_SECOND  (uint32_t)(365 * 24 * 60 * 60)  //平年秒数
#define RTC_DAY_SECOND          (uint32_t)(24 * 60 * 60)        //一天的秒数
#define RTC_HOUR_SECOND         (uint32_t)(60 * 60)             //一个小时的秒数



/* 声明全局函数 */
int8_t RTC_Config(RTC_TimeTypeDef *time);
void RTC_SetClock(RTC_TimeTypeDef *time);
























#endif

