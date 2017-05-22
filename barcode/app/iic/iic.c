#include "IIC.h"
#include "system.h"

#define IIC_DelayUs(x) {SYSTICK_Delay1us(x);}
static void IIC_SendAck(void);
static void IIC_NoAck(void);


/****************************************************************************
* Function Name  : IIC_Config
* Description    : 初始化GPIO.
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void IIC_Config(void)
{
 	RCC->APB2ENR |= 1 << 3;    //先使能外设IO PORTB时钟
	 							 
	GPIOB->CRH &= 0XFFFF00FF;  //PB10/11 推挽输出
	GPIOB->CRH |= 0X00003300;
		   
	GPIOB->ODR |= 3 << 10;     //PB10,11 输出高
}

/****************************************************************************
* Function Name  : IIC_Start
* Description    : IIC发送起始信号.
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void IIC_Start(void)
{
	IIC_SDA_OUT();
	
	IIC_SDA_SET;
	IIC_SCL_SET;
	IIC_DelayUs(5);	

	IIC_SDA_CLR;	//START:when CLK is high,DATA change form high to low
	IIC_DelayUs(5);	//保持时间>4us
	
	IIC_SCL_CLR;    //钳住IIC总线，准备发送或接收数据					
}

/****************************************************************************
* Function Name  : IIC_Stop
* Description    : IIC发送结束信号.
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void IIC_Stop(void)
{
	IIC_SDA_OUT();

	IIC_SCL_CLR;
	IIC_SDA_CLR;	//SCL在低电平期间，SDA变化无效

	IIC_SCL_SET;
	IIC_DelayUs(5);	//保持时间>4us

	IIC_SDA_SET;    //STOP:when CLK is high DATA change form low to high
	IIC_DelayUs(5);	//保持时间>4.7us
}

/****************************************************************************
* Function Name  : IIC_SendData
* Description    : IIC发送一个8位数据
* Input          : dat：发送的数据
* Output         : None
* Return         : None
****************************************************************************/

void IIC_SendData(uint8_t dat)
{
	uint8_t i;
	
	IIC_SDA_OUT();
	IIC_SCL_CLR;

	for(i=0; i<8; i++)//要发送8位，从最高位开始
	{
		if((dat & 0x80) == 0x80)
		{
			IIC_SDA_SET;
		}
		else
		{
			IIC_SDA_CLR;
		}
		dat <<= 1;
		IIC_DelayUs(2);

		IIC_SCL_SET;
		IIC_DelayUs(5);  //保持时间>4.7us
		IIC_SCL_CLR;
		IIC_DelayUs(2);
	}	
}

/****************************************************************************
* Function Name  : IIC_ReceiveData
* Description    : IIC接收一个8位数据.
* Input          : ack：1：产生应答；0：不产生应答
* Output         : None
* Return         : 读取到的数据
****************************************************************************/

uint8_t IIC_ReceiveData(uint8_t ack)
{
	uint8_t i, readValue;

	IIC_SDA_OUT();
	IIC_SDA_SET;	
	IIC_SDA_IN();
	IIC_SCL_CLR;

	for(i=0; i<8; i++)	 //接收8个字节,从高到低
	{
		IIC_SCL_SET;
		IIC_DelayUs(2);
		readValue <<= 1;
		if(IIC_SDA != 0)
		{
			readValue |= 0x01;
		}		
		IIC_DelayUs(1);
		
		IIC_SCL_CLR;
		IIC_DelayUs(5);			
	}
	
	if(ack)		 //是否应答
	{
		IIC_SendAck();
	}
	else
	{
		IIC_NoAck();
	}

	return readValue;
}

/****************************************************************************
* Function Name  : IIC_WaitAck
* Description    : 等待应答.
* Input          : None
* Output         : None
* Return         : 1：应答成功；0：应答失败
****************************************************************************/

int8_t IIC_WaitAck(void)
{
	uint32_t i;

	IIC_SDA_IN();
	IIC_SDA_SET;
	IIC_DelayUs(1);
	IIC_SCL_SET;
	

	while(IIC_SDA)//在SCL高电平的情况下，SDA产生一个下降沿
	{
		 i++;
		 if(i > 0xFFFF)
		 {
		 	return 0xFF;
		 }
	}
	
	IIC_SCL_CLR;
	IIC_DelayUs(2);

	return 0;	
}

/****************************************************************************
* Function Name  : IIC_SendAck
* Description    : 发送应答信号.
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

static void IIC_SendAck(void)
{	
	IIC_SCL_CLR;
	IIC_SDA_OUT();
		
	IIC_SDA_CLR;
	IIC_DelayUs(2);
	IIC_SCL_SET;
	IIC_DelayUs(2);
	IIC_SCL_CLR;		
}

/****************************************************************************
* Function Name  : IIC_NoAck
* Description    : 无应答.
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

static void IIC_NoAck(void)
{	
	IIC_SCL_CLR;
	IIC_SDA_OUT();
		
	IIC_SDA_SET;
	IIC_DelayUs(2);
	IIC_SCL_SET;
	IIC_DelayUs(2);
	IIC_SCL_CLR;		
}


