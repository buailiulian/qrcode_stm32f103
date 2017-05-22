#ifndef __MYFATAPP_H
#define __MYFATAPP_H

#include"stm32f10x.h"
#include"ff.h"
#include"usart.h"

typedef struct
{
	uint8_t type[6];		   //后缀6个字节
	uint8_t name[100];		   //路径和文件名字100个字节（支持25个汉字大小名字）
} FileNameTypeDef;



uint8_t FATFS_GetFree(uint8_t *drv, uint32_t *total, uint32_t *free);
uint8_t FATFS_ScanFiles(uint8_t *dir, FileNameTypeDef *file);
//uint8_t FATFS_ReadFile(void);

#endif
