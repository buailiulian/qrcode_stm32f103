#include"malloc.h"

/* 将所有空间定义位一个数据，首地址从0x68000000开始，并且4字节对齐 */
__align(4) uint8_t MemoryBase[MEMORY_MAX_SIZE] __attribute__((at(0X68000000)));

/* 定义一个内存管理表 */
uint16_t MemoryManagementMap[MAP_TABLE_SIZE] __attribute__((at(0X68000000 + MEMORY_MAX_SIZE)));

/* 定义一个内存状态结构体 ,同时初始化 */
MemoryTypeDef Memory = 
{
	0,						//内存管理状态不可用
	MemoryBase,				//内存池首地址
	MemoryManagementMap,	//内存管理表
	MAP_TABLE_SIZE,			//空闲内存块
};

/****************************************************************************
* Function Name  : MEMORY_Set
* Description    : 设置内存管理状态
* Input          : s：设置内存块的首地址
*                * length：设置的长度
*                * dat：设置的状态
* Output         : None
* Return         : None
****************************************************************************/

static void MEMORY_Set(uint16_t *s, uint32_t length, uint8_t dat)
{
	while(length--)
	{
		*s = dat;
		s++;		
	}	
}

/****************************************************************************
* Function Name  : MEMORY_Init
* Description    : 初始化内存管理表，并标记内存管理程序可用
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void MEMORY_Init(void)
{
	/* 将管理复位清零 */
	MEMORY_Set(MemoryManagementMap, MAP_TABLE_SIZE, 0);
	Memory.managementReady = 1;	
}

/****************************************************************************
* Function Name  : MEMORY_Maloc
* Description    : 查找一个连续的的空间，并返回内存池地址偏移量
* Input          : size：开辟空间的大小
* Output         : None
* Return         : 0xFFFFFFFF：表示开辟失败；其他表示在内存池中的地址偏移量
****************************************************************************/

static uint32_t MEMORY_Maloc(uint32_t size)
{	
	uint16_t block, emptyBlock;
	int32_t addrOffset, j;
	
	/* 若未初始化，先初始化 */
	if(!Memory.managementReady)
	{
		MEMORY_Init();	
	}
	
	if((size == 0) || (size > (Memory.memoryFree * MEMORY_BLOCK_SIZE)))
	{
		return 0xFFFFFFFF;                //无内容或超出长度，返回失败
	}

	block = size / MEMORY_BLOCK_SIZE;	  //求出需要多少个内存块
	if((size % MEMORY_BLOCK_SIZE) != 0)	  //所有还有余数，那么要添加一个内存块
	{
		block++;
	}

	/* 搜索全内存管理表，找到一个连续的内存块，并将地址偏移返回 */
	emptyBlock = 0;
	for(addrOffset=MAP_TABLE_SIZE - 1; addrOffset>=0; addrOffset--)
	{
		if(Memory.managementMap[addrOffset] == 0) 
		{
			emptyBlock++;	
		}
		else
		{
			emptyBlock = 0;
		}
		if(emptyBlock == block)
		{
			
			/* 将内存块标记为使用状态 */
			for(j=0; j<block; j++)
			{	
				/* 将内存块大小写入到管理表的数组中，方便释放时读取内存空间 */
				/* 的大小。 */
				Memory.managementMap[addrOffset + j] = block; 				
			}
			Memory.memoryFree -= block;    //空闲内存块减少 

			/* 偏移地址要*内存块大小才是内存池地址偏移 */
			return (addrOffset * MEMORY_BLOCK_SIZE);
		}			
	}
	
	/* 如果最后扫描完都没有找到合适的内存块，返回失败 */
	return 0xFFFFFFFF;	
}

/****************************************************************************
* Function Name  : MEMORY_Free
* Description    : 释放开辟的内存空间.
* Input          : addrOffset：内存池的地址偏移量
* Output         : None
* Return         : 0：释放完成；0xFF：无内容释放或者释放失败
****************************************************************************/

static uint8_t MEMORY_Free(uint32_t addrOffset)
{
	uint16_t block;

	/* 若未初始化，先初始化 */
	if(!Memory.managementReady)
	{
		MEMORY_Init();
		return 0xFF;	
	}
	if(addrOffset < MEMORY_MAX_SIZE) //在内存池内
	{
		/* 先将内存池地址偏移量转换为内存管理表偏移量 */
		addrOffset = addrOffset / MEMORY_BLOCK_SIZE;

		/* 读取开辟的内存空间大小 */
		block = Memory.managementMap[addrOffset];

		/* 将释放内存的内存管理表释放 */
		MEMORY_Set(&Memory.managementMap[addrOffset], block, 0);
		Memory.memoryFree += block;    //空闲内存块增加
		return 0;
	}
	else
	{
		return 0xFF;	
	}
}

/****************************************************************************
* Function Name  : malloc
* Description    : 开辟一个内存空间
* Input          : size：开辟空间的大小
* Output         : None
* Return         : 开辟空间的地址指针
****************************************************************************/

void *malloc(uint32_t size)
{
	uint32_t addrOffset;

	addrOffset = MEMORY_Maloc(size);  //查找一个合适的内存空间
	
	if(addrOffset == 0xFFFFFFFF)      //分配失败
	{
		return 0;				      //返回NULL（即0）
	}
	
	/* 将地址偏移量转换成指针地址返回 */
	return (void *)((uint32_t)Memory.memoryBaseAddr + addrOffset);	
}

/****************************************************************************
* Function Name  : free
* Description    : 释放指定的内存空间
* Input          : pro：要释放的内存空间指针地址
* Output         : None
* Return         : None
****************************************************************************/

void free(void *pro)
{
	uint32_t addrOffset;

	if(pro == 0)	   //若指针为NULL的时候
	{
		return;
	}

	/* 读取该内存空间在内存池上面的地址偏移量 */
	addrOffset = (uint32_t)pro - (uint32_t)Memory.memoryBaseAddr;

	/* 释放内存 */
	MEMORY_Free(addrOffset);

	/* 释放pro指针 */
	pro = 0;
}

//void MEMORY_Copy(uint16_t cut, uint16_t paste)
//{
//	uint16_t i;
//	uint32_t cutOffset, pasteOffset, mapData;
//	
//	//-- 读取内存管理表中内存块的大小 --//
//	mapData = Memory.managementMap[cut];
//
//	//-- 将内存块偏移地址转为内存池偏移地址	--//
//	cutOffset = cut * MEMORY_BLOCK_SIZE;	 
//	pasteOffset = paste * MEMORY_BLOCK_SIZE;
//	
//	//-- 复制内存池中的数据,一个内存块一共有MEMORY_BLOCK_SIZE数据--//
//	for(i=0; i<MEMORY_BLOCK_SIZE; i++)
//	{
//		MemoryBase[pasteOffset] = MemoryBase[cutOffset];
//		pasteOffset++;
//		cutOffset++;
//	}
//
//	//-- 将内存管理表中的状态切换 --//
//	Memory.managementMap[cut] = 0;
//	Memory.managementMap[paste] = mapData;
//}
//void MEMORY_Optimization(void)
//{
//	uint32_t emptyBlock;
//	int32_t block;
//
//	for(block=MAP_TABLE_SIZE - 1; block>=0; block--)
//	{
//		if(Memory.managementMap[block] == 0)
//		{
//			emptyBlock = block;	// 读取内存块的偏移量
//			while(block >= 0)
//			{
//				if(Memory.managementMap[block] != 0)
//				{
//					MEMORY_Copy(block, emptyBlock);
//					emptyBlock--;		 
//				}
//				block--;	
//			}
//		}			
//	}
//}




