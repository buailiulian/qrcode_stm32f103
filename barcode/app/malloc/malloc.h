#ifndef __MALLOC_H
#define __MALLOC_H

/* 包含适用的头文件 */
#include"stm32f10x.h"

/* 定义内存管理的总内存管理长度，IS62WV512有1MB字节，但是需要一部分用来存管 */
/* 理列表一定要小于1MB。  */
#define MEMORY_MAX_SIZE   200*1024  		//先设置为200KB

/* 定义的管理表 */
#define MEMORY_BLOCK_SIZE 32                //内存块长度

/* 内存管理表的长度 = 总内存管理长度（MEMORY_MAX_SIZE） / 32。 */
#define MAP_TABLE_SIZE    (MEMORY_MAX_SIZE / 32)


/* 定义一个结构体，用来保存内存状态 */

typedef struct 
{
	uint8_t managementReady;	   //内存管理状态	
	uint8_t *memoryBaseAddr;       //内存池首地址
	uint16_t *managementMap;	   //内存管理表
	uint16_t memoryFree;		   //空闲内存块(没有用到的内存)

} MemoryTypeDef;

/* 定义一个内存状态结构体 */
extern MemoryTypeDef Memory;

/* 声明全局函数（外部调用）*/
void MEMORY_Init(void); 
void *malloc(uint32_t size);
void free(void *pro);






#endif
