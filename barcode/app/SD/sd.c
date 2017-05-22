#include "sd.h"
#include "spi.h"

/* 定义全局变量 */
uint8_t SD_TYPE;	  //保存SD卡的类型

/****************************************************************************
* Function Name  : SD_GPIO_Config
* Description    : 初始化SD使用的IO和SPI.
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

static void SD_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOG, ENABLE);
	
	/* SD_CS PG14/ FLASH_CS PG13 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_SetBits(GPIOG, GPIO_Pin_14);
	GPIO_SetBits(GPIOG, GPIO_Pin_13);
	
	/* ENC28J60_CS PB12 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_12); //避免影响SD卡

	SPI2_Config();
}

/****************************************************************************
* Function Name  : SD_WriteCmd
* Description    : 向SD卡发送一个命令.SD卡一个命令格式包括8位命令，
*                * 32位命令参数和7位CRC效验位，最后一位为1
* Input          : cmd：命令
*                * dat: 命令参数
*                * crc: CRC效验
* Output         : None
* Return         : r1：SD卡返回的数据
*                * 0xFF：表示发送失败
****************************************************************************/

static uint8_t SD_WriteCmd(uint8_t cmd, uint32_t dat, uint8_t crc)
{	 
	uint8_t r1 = 0;
	uint16_t i = 0;

	//--复位SD卡,取消上次片选--//
	SD_CS_SET;
	SPI2_WriteReadData(0xFF);	 //额外提供8个时钟

	SD_CS_CLR;
	while(SPI2_WriteReadData(0xFF) != 0xFF) //等待卡是否准备好
	{
		i++;
		if(i > 100)
		{
			return 0xFF;	                 //等待失败返回
		}
	}

	//--发送数据--//
	SPI2_WriteReadData(cmd | 0x40);

	SPI2_WriteReadData(dat >> 24);     //发送Dat的最高8位
	SPI2_WriteReadData(dat >> 16);
	SPI2_WriteReadData(dat >> 8);
	SPI2_WriteReadData(dat & 0x00FF);

	SPI2_WriteReadData(crc);
	if(cmd == SD_CMD12)		 //如果是停止数据传输命令，额外多发一个时钟
	{
		SPI2_WriteReadData(0xFF);
	}

	i = 0;
	do
	{
		r1 = SPI2_WriteReadData(0xFF);	
		i++;
		if(i > 100)
		{
			return 0xFF;
		}
	}
	while((r1 & 0x80) != 0);   //发送成功的最高位是0 

	return r1;
}

/****************************************************************************
* Function Name  : SD_Init
* Description    : 初始化SD卡.
* Input          : None
* Output         : None
* Return         : 0xFF：初始化失败
*		         * 0：初始化成功
****************************************************************************/

int8_t SD_Init(void)
{
	uint8_t r1, buf[4];
	uint16_t i = 0;

	SD_GPIO_Config();
	
	SPI2_SetSpeed(SPI_BaudRatePrescaler_256);
	//--将SD卡通信模式转换为SPI模式，上电默认是用DAT0作数据线--//
	//--接到CMD0时，CS信号有效，SPI模式启动--//
	for(i=0; i<0x0F; i++)//初始时，先发送至少74个时钟，这个是必须的。
	{
		SPI2_WriteReadData(0xFF);	 //发送8*16个
	}

	//当读取到0x01的时候表示初始化成功
	i = 0;
	while(SD_WriteCmd(SD_CMD0, 0, 0x95) != 0x01)
	{
		i++;
		if(i > 100)
		{
			return 0xFF;        //初始化失败返回0
		}
	}
		
	//--发送CMD8，检测是否SD V2.0--//
	i = 0;
	do
	{
		i++;
		if(i > 100)	//若是发送超过次数跳出循环管
		{
			break;
		}
		r1 = SD_WriteCmd(SD_CMD8, 0x01AA, 0x87);
	}
	while(r1 != 0x01); //发送CMD8
	
	if(r1 == 0x01)     //如果CMD8有回应说明是SD V2.0协议
	{
		//--读取CMD8的返回值，检测是否支持电压--//
		//--读取CMD8的返回值，检测是否支持电压--//
		for(i=0; i<4; i++)
		{
			buf[i] = SPI2_WriteReadData(0xFF);
		}
		
		//--卡电压不支持电压，返回错误--//
		if((buf[2] != 0x01) || (buf[3] != 0xAA))
		{
			return 0xFF;
		}
		
		//--初始化SD卡--//
		i = 0;
		do
		{
			i++;
			if(i > 100)
			{
				return 0xFF;   //返回失败
			}

			SD_WriteCmd(SD_CMD55, 0, 0x01);
			r1 = SD_WriteCmd(SD_CMD41, 0x40000000, 0x01);
		}
		while(r1 != 0);

		//--检测是SDHC卡还是SD卡--//
		i = 0;
		while(SD_WriteCmd(SD_CMD58, 0, 0x01) != 0)
		{
			i++;
			if(i > 100)
			{
				SD_TYPE = SD_TYPE_ERR;
				break;
			}		
		}

		//--读取OCR--//
		for(i=0; i<4; i++)
		{
			buf[i] = SPI2_WriteReadData(0xFF);
		}

		if(buf[0] & 0x40) 
		{
			SD_TYPE = SD_TYPE_V2HC;
		}
		else
		{
			SD_TYPE = SD_TYPE_V2;
		}	
	}
			
	else //否则就是SD V1.0或者MMC V3
	{
		SD_WriteCmd(SD_CMD55, 0x00, 0x01);
		r1 = SD_WriteCmd(SD_CMD41, 0x00, 0x01);

		if(r1 <= 1)	   //对CMD41有回应说明是SD V1.0
		{
			SD_TYPE = SD_TYPE_V1;	 //是V1.0卡
			i = 0;
			do
			{
				if(i > 100)
				{
					return 0xFF;
				}

				SD_WriteCmd(SD_CMD55, 0x00, 0x01);
				r1 = SD_WriteCmd(SD_CMD41, 0x00, 0x01);	
			}
			while(r1 != 0);	
		}

		else		  //没有回应说明是MMC V3
		{
			SD_TYPE = SD_TYPE_MMC;	 //卡类型是MMC卡
			i = 0;
			while(SD_WriteCmd(SD_CMD1, 0, 0x01) != 0)
			{
				i++;
				if(i > 100)
				{
					return 0xFF;
				}
			}	
		}
	}		

	SD_CS_SET;		   //取消片选
	SPI2_WriteReadData(0xFF); 
//	SPI2_SetSpeed(SPI_BaudRatePrescaler_2);

	return 0;
}

/****************************************************************************
* Function Name  : SD_ReadCapacity
* Description    : 读取SD卡的内存大小.
* Input          : capacity：读取到的数值保存指针
* Output         : 内存大小，单位为KB
* Return         : 0xFF：初始化失败
*		         * 0：初始化成功
****************************************************************************/

int8_t SD_ReadCapacity(uint32_t *capacity)
{
	uint8_t csdValue[16];
	uint16_t n, i = 0;

	//--发送命令--//
	while(SD_WriteCmd(SD_CMD9, 0, 0x01) != 0);
	{
		i++;
		if(i > 100)
		{
			return 0xFF;  //发送命令失败
		}
	}

	//--等待起始令牌0XFE--//
	i = 0;
	while(SPI2_WriteReadData(0xFF) != 0xFE)
	{
		i++;
		if(i > 500)
		{
			return 0xFF;
		}
	}

		
	//--读取数据--//
	for(i=0; i<16; i++)
	{
		csdValue[i] = SPI2_WriteReadData(0xFF);
	}

	//--读取两位CRC效验--//
	SPI2_WriteReadData(0xFF);	  //RCC
	SPI2_WriteReadData(0xFF);
	
	//--取消片选--//
	SD_CS_SET;			  
	SPI2_WriteReadData(0xFF);

	//--SD V2.0的卡CSD第一个数据是0x40--//
	if((csdValue[0] & 0xC0) == 0x40)
	{
		//--计算C_SIZE，在CSD[69:48]--//
		*capacity = csdValue[9] + ((uint16_t)csdValue[8] << 8) + 1;

		//--实际上就是乘以1024--//
		*capacity = (*capacity) << 10;//得到扇区数			
	}
	else   
	{
		//--内存算法是 capacity = BLOCKNR * BLOCK_LEN--//
		//--BLOCKNR = (C_SIZE + 1) * MULT；--//
		//--BLOCK_LEN = (READ_BL_LEN < 12) 或2^(READ_BL_LEN)--//

		//--计算BLOCK_LEN,C_SIZE_MULT在CSD[49:47];READ_BL_LEN在CSD[83:80]--//
		n = (csdValue[5] & 0x0A) + ((csdValue[10] & 0x80) >> 7) 
			 + ((csdValue[9] & 0x03) << 1) + 2;

		//--计算C_SIZE，C_SIZE在CSD[73:62]--//
		*capacity = (csdValue[8] >> 6) + ((uint16_t)csdValue[7] << 2) 
		            + ((uint16_t)(csdValue[6] & 3) << 10) + 1;
		*capacity = (*capacity) << (n - 9);//得到扇区数	
	}

	return 0;		
}

/****************************************************************************
* Function Name  : SD_ReadData
* Description    : 读取512个数据数据.
* Input          : *buf：数据缓存区
* Output         : 读取到的数值
* Return         : 0xFF：初始化失败
*		         * 0：初始化成功
****************************************************************************/

static int8_t SD_ReadData(uint8_t *buf)
{
	uint16_t i;

	//--等待起始令牌0XFE--//
	i = 0;
	while(SPI2_WriteReadData(0xFF) != 0xFE)
	{
		i++;
		if(i > 0x0FFF)
		{
			return 0xFF;
		}
	}
	
	//--接收数据--//
	for(i=0; i<512; i++)
	{
		*buf = SPI2_WriteReadData(0xFF);
		buf++;	
	}

	//--读完数据再读两位CRC效验，但是我们可以不需要它们--//
	SPI2_WriteReadData(0xFF);
	SPI2_WriteReadData(0xFF);
	
	return 0;					  	
}
/****************************************************************************
* Function Name  : SD_ReadDisk
* Description    : 读取扇区的数据.
* Input          : *buf：数据缓存区
*                * sector：读取扇区位置
*                * num：读取的扇区数
* Output         : 读取到的扇区数值
* Return         : 0xFF：初始化失败
*		         * 0：初始化成功
****************************************************************************/

int8_t SD_ReadDisk(uint8_t *buf, uint32_t sector, uint8_t num)
{
	uint16_t i;

	if(SD_TYPE != SD_TYPE_V2HC)
	{
		sector <<= 9; //转换位字节地址
	}
	SPI2_SetSpeed(SPI_BaudRatePrescaler_2);
	if(num == 1)
	{
		//--发送读取命令--//
		i = 0;
		while(SD_WriteCmd(SD_CMD17, sector, 0x01) != 0); 
		{
			i++;
			if(i > 100)
			{
				return 0xFF;  //命令无反应，表明发送命令失败
			}	
		}

		//-- 接收数据 --//
		if(SD_ReadData(buf) != 0)
		{
			return 0xFF;
		}		
	}
	else
	{
		//--发送连续读取命令--//
		i = 0;
		while(SD_WriteCmd(SD_CMD18, sector, 0x01) != 0); 
		{
			i++;
			if(i > 100)
			{
				return 0xFF;  //命令无反应，表明发送命令失败
			}	
		}
		
		//--接收数据--//
		while(num--)
		{
			//-- 接收数据 --//
			if(SD_ReadData(buf) != 0)
			{
				return 0xFF;
			}
			buf += 512;			
		}
		SD_WriteCmd(SD_CMD12, 0, 0x01); //发送停止信号
	}
	
	//--取消片选--//
	SD_CS_SET;
	SPI2_WriteReadData(0xFF);	

	return 0;
}

/****************************************************************************
* Function Name  : SD_WriteData
* Description    : 写512个数据数据.
* Input          : *buf：数据缓存区
*                * cmd：命令
* Output         : None
* Return         : 0xFF：初始化失败
*		         * 0：初始化成功
****************************************************************************/

static int8_t SD_WriteData(uint8_t *buf, uint8_t cmd)
{
	uint16_t i;

	//-- 发送若干个时钟，等待SD卡准备好 --//
	i = 0;
	while(SPI2_WriteReadData(0xFF) != 0xFF)
	{
		if(i > 0x0FFF)
		{
			return 0xFF;	
		}	
	}

	//--发送命令--//
	SPI2_WriteReadData(cmd);

	//--开始写入数据--//
	for(i = 0; i<512; i++)
	{
		SPI2_WriteReadData(*buf);
		buf++;
	}

	//--发送两位CRC效验码，随便发--//
	SPI2_WriteReadData(0xFF);
	SPI2_WriteReadData(0xFF);

	//--读取返回值--//
	i = SPI2_WriteReadData(0xFF);

	if((i & 0x1F) != 0x05)	  //判断是否写成功
	{
	 	SD_CS_SET;
		SPI2_WriteReadData(0xFF);
		return 0xFF;
	}

	return 0;
}
/****************************************************************************
* Function Name  : SD_WriteDisk
* Description    : 写多个扇区.
* Input          : *buf：数据缓存区
*                * sector：要写的扇区数值
*                * 要写的扇区数
* Output         : None
* Return         : 0xFF：初始化失败
*		         * 0：初始化成功
****************************************************************************/

int8_t SD_WriteDisk(uint8_t *buf, uint32_t sector, uint8_t num)
{
	uint8_t i;

	if(SD_TYPE != SD_TYPE_V2HC)
	{
		sector <<= 9; //转换位字节地址
	}
	SPI2_SetSpeed(SPI_BaudRatePrescaler_2);
	//--- 只写一个扇区 ---//
	if(num == 1)
	{
		//--发送写命令--//
		i = 0;
		while(SD_WriteCmd(SD_CMD24, sector, 0x01) != 0); 
		{
			i++;
			if(i > 100)
			{
				return 0xFF;  //命令无反应，表明发送命令失败
			}	
		}
	
		if(SD_WriteData(buf, 0xFE) != 0)
		{
			return 0xFF;	
		}
	}

	//--- 写多个扇区 ---//
	else
	{
		if(SD_TYPE == SD_TYPE_MMC)		  //如果是MMC卡
		{
			SD_WriteCmd(SD_CMD55, 0, 0X01);	
			SD_WriteCmd(SD_CMD23, num, 0X01); //写入前先擦除num个扇区里面的数据	
		}
		//--发送连续写命令--//
		i = 0;
		while(SD_WriteCmd(SD_CMD25, sector, 0x01) != 0); 
		{
			i++;
			if(i > 100)
			{
				return 0xFF;  //命令无反应，表明发送命令失败
			}	
		}

		//--- 开始写数据 ---//
		while(num--)
		{
			if(SD_WriteData(buf, 0xFC) != 0)
			{
				return 0xFF;	
			}
			buf += 512;	
		}

		//-- 发送若干个时钟，等待SD卡准备好 --//
		i = 0;
		while(SPI2_WriteReadData(0xFF) != 0xFF)
		{
			if(i > 0x0FFF)
			{
				return 0xFF;	
			}	
		}
	
		//--发送结束命令--//
		SPI2_WriteReadData(0xFD);

	}

	SD_CS_SET;
	SPI2_WriteReadData(0xFF);

	return 0;
}
