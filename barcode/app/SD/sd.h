#ifndef __SD_H
#define __SD_H



#include "stm32f10x.h"

/* 添加全局变量 */
extern uint8_t SD_TYPE;	   //保存SD卡的类型

/* 定义卡的类型 */
#define SD_TYPE_ERR     ((uint8_t)0X00)
#define SD_TYPE_MMC     ((uint8_t)0X01)
#define SD_TYPE_V1      ((uint8_t)0X02)
#define SD_TYPE_V2      ((uint8_t)0X04)
#define SD_TYPE_V2HC    ((uint8_t)0X06)

/* 定义IO口 */
#define SD_CS_SET GPIO_SetBits(GPIOG, GPIO_Pin_14)
#define SD_CS_CLR GPIO_ResetBits(GPIOG, GPIO_Pin_14)

/* SD卡指令表 */  	   
#define SD_CMD0    0       //卡复位
#define SD_CMD1    1
#define SD_CMD8    8       //命令8 ，SEND_IF_COND
#define SD_CMD9    9       //命令9 ，读CSD数据
#define SD_CMD10   10      //命令10，读CID数据
#define SD_CMD12   12      //命令12，停止数据传输
#define SD_CMD16   16      //命令16，设置SectorSize 应返回0x00
#define SD_CMD17   17      //命令17，读sector
#define SD_CMD18   18      //命令18，读Multi sector
#define SD_CMD23   23      //命令23，设置多sector写入前预先擦除N个block
#define SD_CMD24   24      //命令24，写sector
#define SD_CMD25   25      //命令25，写Multi sector
#define SD_CMD41   41      //命令41，应返回0x00
#define SD_CMD55   55      //命令55，应返回0x01
#define SD_CMD58   58      //命令58，读OCR信息
#define SD_CMD59   59      //命令59，使能/禁止CRC，应返回0x00

/* 声明全局函数 */
int8_t SD_Init(void);
int8_t SD_ReadCapacity(uint32_t *capacity);
int8_t SD_ReadDisk(uint8_t *buf, uint32_t sector, uint8_t num);
int8_t SD_WriteDisk(uint8_t *buf, uint32_t sector, uint8_t num);











#endif
