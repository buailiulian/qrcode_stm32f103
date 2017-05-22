#include "eeprom.h"

/* 声明全局变量 */
static int8_t AT24C02_WriteData(uint8_t *writeBuff, uint8_t writeAddr, uint8_t writeNumByte);
static void AT24C02_WaitEepromStandbyState(void);
      
/* 使用硬件I2C */
#ifdef USE_I2C_HARDWARE

/****************************************************************************
* Function Name  : AT24C02_Init
* Description    : 初始化GPIO和硬件I2C
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void AT24C02_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef  I2C_InitStructure;

	/* 打开时钟使能 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	/* 设置GPIO的参数 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;

	GPIO_Init(GPIOB, &GPIO_InitStructure);

/***************************************************************************/
/********************* 硬件I2C设置并初始化 *********************************/
/***************************************************************************/
	/* 打开时钟使能 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

	/* 设置I2C的参数 */
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;        //设置为I2C模式
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;//时钟占空比
	I2C_InitStructure.I2C_OwnAddress1 = 0x10;		  //设置自己的地址，随意取
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	/* 7位应答地址 */
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 200000;        //不大于400KHZ

	I2C_Cmd(I2C2, ENABLE);	
	I2C_Init(I2C2, &I2C_InitStructure);
}

/****************************************************************************
* Function Name  : AT24C02_WriteData
* Description    : 写入24C02的数据.注意：地址自动加1只能在同一个扇区
* Input          : writeBuff：写入数据的保存地址
*                * writeAddr：写入的地址 
*                * writeNumByte：写入数据长度（24C02的writeNumByte <= 8）
* Output         : None
* Return         : 0：成功；0xFF：失败。
****************************************************************************/

static int8_t AT24C02_WriteData(uint8_t *writeBuff, uint8_t writeAddr, uint8_t writeNumByte)
{
	uint16_t i = 0;

	/* 检测I2C总线是否忙碌 */
	while (I2C_GetFlagStatus(I2C2, I2C_FLAG_BUSY))
	{
		i++;
		if(i == 100000)
		{
			return 0xFF;
		}
	}

	/* 发送起始信号 */
	I2C_GenerateSTART(I2C2, ENABLE);

	i = 0;
	/* 检测EV5，即是否启动为主机模式 */
	while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
	{
		i++;
		if(i == 100000)
		{
			return 0xFF;
		}
	}

	/* 发送写器件地址 */
	I2C_Send7bitAddress(I2C2, EEPROM_ADDR, I2C_Direction_Transmitter);//发送模式

	i = 0;
	/* 检测EV6并清除，*/
	while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		i++;
		if(i == 100000)
		{
			return 0xFF;
		}
	}

	/* 发送写地址 */
	I2C_SendData(I2C2, writeAddr);

	i = 0;
	/* 检测EV8 */
	while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		i++;
		if(i == 100000)
		{
			return 0xFF;
		}
	}

	while (writeNumByte--)
	{
		/* 发送数据 */
		I2C_SendData(I2C2, *writeBuff);
		
		i = 0;
		/* 检测EV8 */
		while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
			i++;
			if(i == 100000)
			{
				return 0xFF;
			}
		}

		writeBuff++;
	}

	I2C_GenerateSTOP(I2C2, ENABLE);

	return 0;
}

/****************************************************************************
* Function Name  : AT24C02_WaitEepromStandbyState
* Description    : 等待24C02写入成功.
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

static  void AT24C02_WaitEepromStandbyState(void)      
{
	uint16_t i = 0;
	__IO uint16_t SR1_Tmp = 0;
	
	do
	{
		/* Send START condition */ 
		I2C_GenerateSTART(I2C2, ENABLE);
		/*  Read I2C2 SR1 register */ 
		SR1_Tmp = I2C_ReadRegister(I2C2, I2C_Register_SR1);
		/* Send EEPROM address for write */ 
		I2C_Send7bitAddress(I2C2, EEPROM_ADDR, I2C_Direction_Transmitter);

		i++;
		if(i == 100000)
		{
			return ;
		}
	}
	while(!(I2C_ReadRegister(I2C2, I2C_Register_SR1) & 0x0002));
	
	/* Clear AF flag */ 
	I2C_ClearFlag(I2C2, I2C_FLAG_AF);
	
	/* STOP condition */ 
	I2C_GenerateSTOP(I2C2, ENABLE);  
}

/****************************************************************************
* Function Name  : AT24C02_WriteBuff
* Description    : 写入24C02的数据
* Input          : writeBuff：写入数据的保存地址
*                * writeAddr：写入的地址 
*                * writeNumByte：写入数据长度
* Output         : None
* Return         : 0：成功；0xFF：失败。
****************************************************************************/

int8_t AT24C02_WriteBuff(uint8_t *writeBuff, uint8_t writeAddr, uint8_t writeNumByte)
{
	uint8_t addr = 0, buffNum = 0, byteNum = 0;

	addr = writeAddr % EEPROM_PAGE_SIZE;    //24c02一次8位一个扇区		

	if(addr ==0)              //当写入地址正好等于一个扇区的起始地址
	{
		buffNum = writeNumByte / EEPROM_PAGE_SIZE;	    //求出扇区数  
		byteNum = writeNumByte % EEPROM_PAGE_SIZE;		//求扇区余数

        /* 先写整扇区的数据 */
		while(buffNum--)  //先写入完整扇区
		{
			if(AT24C02_WriteData(writeBuff, writeAddr, EEPROM_PAGE_SIZE))
			{
				return 0xFF;
			}
			AT24C02_WaitEepromStandbyState();
			
			writeBuff += EEPROM_PAGE_SIZE;
			writeAddr += EEPROM_PAGE_SIZE; 		
		}
		
        /* 写完完整扇区，开始写超出完整扇区的字节 */ 
		if(byteNum != 0)  
		{
			if(AT24C02_WriteData(writeBuff, writeAddr, byteNum))
			{
				return 0xFF;
			}
			AT24C02_WaitEepromStandbyState();
		}
	
	}
	else  //else of if(addr ==0) 
	{
		addr = EEPROM_PAGE_SIZE - addr;            //求出有多少数据就到整扇区地址
        if(writeNumByte > addr)
        {
            writeNumByte = writeNumByte - addr;    //如果写入数据大于扇区剩余地址
            buffNum = writeNumByte / EEPROM_PAGE_SIZE; //求出扇区数  
		    byteNum = writeNumByte % EEPROM_PAGE_SIZE; //求扇区余数	
		}
        
        /* 先写入前面不到整扇区地址的数据 */		
		if(AT24C02_WriteData(writeBuff, writeAddr, addr))
		{
			return 0xFF;
		}
		AT24C02_WaitEepromStandbyState();
		writeAddr += addr;     //写完地址添加
        writeBuff += addr;     //数据地址后移

        /* 再写入完整扇区 */
		while(buffNum--)  
		{
			if(AT24C02_WriteData(writeBuff, writeAddr, EEPROM_PAGE_SIZE))
			{
				return 0xFF;
			}
			AT24C02_WaitEepromStandbyState();
			
			writeBuff += EEPROM_PAGE_SIZE;
			writeAddr += EEPROM_PAGE_SIZE;	
		}
		
        /* 写完完整扇区，开始写超出完整扇区的字节 */
		if(byteNum != 0)  
		{
			if(AT24C02_WriteData(writeBuff, writeAddr, byteNum))
			{
				return 0xFF;
			}
			AT24C02_WaitEepromStandbyState();
		}		
	}

    AT24C02_WaitEepromStandbyState();

	return 0;
}

/****************************************************************************
* Function Name  : AT24C02_ReadBuff
* Description    : 读取24C02的数据.
* Input          : readBuff：读取到数据的保存地址
*                * readAddr：读取的地址 
*                * readNumByte：读取的数据长度
* Output         : None
* Return         : 0：成功；0xFF：失败。
****************************************************************************/

int8_t AT24C02_ReadBuff(uint8_t *readBuff, uint8_t readAddr, uint8_t readNumByte)
{
	uint16_t i = 0;

	/* 检测I2C总线是否忙碌 */
	while (I2C_GetFlagStatus(I2C2, I2C_FLAG_BUSY))
	{
			i++;
			if(i == 100000)
			{
				return 0xFF;
			}
	}

	/* 发送起始信号 */
	I2C_GenerateSTART(I2C2, ENABLE);

	i = 0;
	/* 检测EV5，即是否启动为主机模式 */
	while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
	{
		i++;
		if(i == 100000)
		{
			return 0xFF;
		}
	}
	
	/* 发送写器件地址 */
	I2C_Send7bitAddress(I2C2, EEPROM_ADDR, I2C_Direction_Transmitter);//发送模式

	i = 0;
	/* 检测EV6并清除，*/
	while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		i++;
		if(i == 100000)
		{
			return 0xFF;
		}
	}

	/* Clear EV6 by setting again the PE bit */
  	I2C_Cmd(I2C2, ENABLE);

	/* 发送读取地址 */
	I2C_SendData(I2C2, readAddr);

	i = 0;
	/* Test on EV8 and clear it */
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		i++;
		if(i == 100000)
		{
			return 0xFF;
		}
	}

	/* 发送起始信号 */
	I2C_GenerateSTART(I2C2, ENABLE);

	i = 0;
	/* 检测EV5，*/
	while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
	{
		i++;
		if(i == 100000)
		{
			return 0xFF;
		}
	}

	/* 发送读器件地址 */
    I2C_Send7bitAddress(I2C2, EEPROM_ADDR, I2C_Direction_Receiver);

	i = 0;
    /* Test on EV6 and clear it */
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
		i++;
		if(i == 100000)
		{
			return 0xFF;
		}
	}
 
	while (readNumByte)
	{
		if(readNumByte==1)
		{
			/* 失能应答 */
			I2C_AcknowledgeConfig(I2C2, DISABLE);
			/* 发送停止条件 */
			I2C_GenerateSTOP(I2C2, ENABLE);
		}

		/* 接收数据 */
		while(I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED))
		{
			/* 读取接收到的数据 */
			*readBuff = I2C_ReceiveData(I2C2);

			readBuff++;
			readNumByte--;
		}
	}/* end of while(readNumByte); */

	I2C_AcknowledgeConfig(I2C2, ENABLE);
	AT24C02_WaitEepromStandbyState();

	return 0;
}

#else

/****************************************************************************
* Function Name  : AT24C02_WriteData
* Description    : 写入24C02的数据.注意：地址自动加1只能在同一个扇区
* Input          : writeBuff：写入数据的保存地址
*                * writeAddr：写入的地址 
*                * writeNumByte：写入数据长度（24C02的writeNumByte <= 8）
* Output         : None
* Return         : 0：成功；0xFF：失败。
****************************************************************************/

static int8_t AT24C02_WriteData(uint8_t *writeBuff, uint8_t writeAddr, uint8_t writeNumByte)
{
	IIC_Start();
	IIC_SendData(EEPROM_ADDR); //24C02写地址
	if(IIC_WaitAck())
    {
        return 0xFF;
    }
	IIC_SendData(writeAddr);   //数据写入地址
	if(IIC_WaitAck())
    {
        return 0xFF;
    }

    while(writeNumByte--)
    {
    	IIC_SendData(*writeBuff);
    	if(IIC_WaitAck())
        {
            return 0xFF;
        }
        writeBuff++;
    }
	IIC_Stop();

	return 0;
}

/****************************************************************************
* Function Name  : AT24C02_WaitEepromStandbyState
* Description    : 等待24C02写入成功.
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

static  void AT24C02_WaitEepromStandbyState(void)      
{
    uint32_t i = 0;
    do
    {
        IIC_Start();
	    IIC_SendData(EEPROM_ADDR); //24C02写地址
        i++;
        if(i > 0x00FFFF)
        {
            return;
        }
	}
    while(IIC_WaitAck());   
}
/****************************************************************************
* Function Name  : AT24C02_WriteBuff
* Description    : 写入24C02的数据
* Input          : writeBuff：写入数据的保存地址
*                * writeAddr：写入的地址 
*                * writeNumByte：写入数据长度
* Output         : None
* Return         : 0：成功；0xFF：失败。
****************************************************************************/

int8_t AT24C02_WriteBuff(uint8_t *writeBuff, uint8_t writeAddr, uint8_t writeNumByte)
{
	uint8_t addr = 0, buffNum = 0, byteNum = 0;

	addr = writeAddr % EEPROM_PAGE_SIZE;    //24c02一次8位一个扇区		

	if(addr ==0)              //当写入地址正好等于一个扇区的起始地址
	{
		buffNum = writeNumByte / EEPROM_PAGE_SIZE;	    //求出扇区数  
		byteNum = writeNumByte % EEPROM_PAGE_SIZE;		//求扇区余数

        /* 先写整扇区的数据 */
		while(buffNum--)  //先写入完整扇区
		{
			if(AT24C02_WriteData(writeBuff, writeAddr, EEPROM_PAGE_SIZE))
			{
				return 0xFF;
			}
			AT24C02_WaitEepromStandbyState();
			
			writeBuff += EEPROM_PAGE_SIZE;
			writeAddr += EEPROM_PAGE_SIZE; 		
		}
		
        /* 写完完整扇区，开始写超出完整扇区的字节 */ 
		if(byteNum != 0)  
		{
			if(AT24C02_WriteData(writeBuff, writeAddr, byteNum))
			{
				return 0xFF;
			}
			AT24C02_WaitEepromStandbyState();
		}
	
	}
	else  //else of if(addr ==0) 
	{
		addr = EEPROM_PAGE_SIZE - addr;            //求出有多少数据就到整扇区地址
        if(writeNumByte > addr)
        {
            writeNumByte = writeNumByte - addr;    //如果写入数据大于扇区剩余地址
            buffNum = writeNumByte / EEPROM_PAGE_SIZE; //求出扇区数  
		    byteNum = writeNumByte % EEPROM_PAGE_SIZE; //求扇区余数	
		}
        
        /* 先写入前面不到整扇区地址的数据 */		
		if(AT24C02_WriteData(writeBuff, writeAddr, addr))
		{
			return 0xFF;
		}
		AT24C02_WaitEepromStandbyState();
		writeAddr += addr;     //写完地址添加
        writeBuff += addr;     //数据地址后移

        /* 再写入完整扇区 */
		while(buffNum--)  
		{
			if(AT24C02_WriteData(writeBuff, writeAddr, EEPROM_PAGE_SIZE))
			{
				return 0xFF;
			}
			AT24C02_WaitEepromStandbyState();
			
			writeBuff += EEPROM_PAGE_SIZE;
			writeAddr += EEPROM_PAGE_SIZE;	
		}
		
        /* 写完完整扇区，开始写超出完整扇区的字节 */
		if(byteNum != 0)  
		{
			if(AT24C02_WriteData(writeBuff, writeAddr, byteNum))
			{
				return 0xFF;
			}
			AT24C02_WaitEepromStandbyState();
		}		
	}

    AT24C02_WaitEepromStandbyState();

	return 0;
}

/****************************************************************************
* Function Name  : AT24C02_ReadBuff
* Description    : 读取24C02的数据.
* Input          : readBuff：读取到数据的保存地址
*                * readAddr：读取的地址 
*                * readNumByte：读取的数据长度
* Output         : None
* Return         : 0：成功；0xFF：失败。
****************************************************************************/

int8_t AT24C02_ReadBuff(uint8_t *readBuff, uint8_t readAddr, uint8_t readNumByte)
{

	IIC_Start();
	IIC_SendData(EEPROM_ADDR);   //写地址
	if(IIC_WaitAck())
    {
        return 0xFF;
    }
	IIC_SendData(readAddr);
	if(IIC_WaitAck())
    {
        return 0xFF;
    }

	IIC_Start();
	IIC_SendData(EEPROM_ADDR + 1);
	if(IIC_WaitAck())
    {
        return 0xFF;
    }
	while(readNumByte)
	{   
        readNumByte--;
		if(readNumByte != 0)       //未接受完成就发送应答
		{
			*readBuff = IIC_ReceiveData(1);
		}
        else                      //接受完成就发送非应答
        {
            *readBuff = IIC_ReceiveData(0);
        }
        readBuff++;
	}

	IIC_Stop();
	return 0;
}

#endif  //end of #ifndef USE_I2C_HARDWARE

