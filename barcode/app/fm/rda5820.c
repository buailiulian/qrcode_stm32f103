#include "rda5820.h"
#include "system.h"
#include "iic.h"

/****************************************************************************
* Function Name  : RDA5820_DelayMs
* Description    : RDA5820延时函数
* Input          : ms：延时时间
* Output         : None
* Return         : None
****************************************************************************/
#define RDA5820_DelayMs(x) {SYSTICK_Delay1ms(x);}

/****************************************************************************
* Function Name  : RDA5820_ReadData
* Description    : RDA5820使用IIC读取一个16位数据
* Input          : addr：读取寄存器地址
* Output         : None
* Return         : dat：读取到的数据
****************************************************************************/
		
static uint16_t RDA5820_ReadData(uint8_t addr)
{
	uint16_t dat;
	
	IIC_Start();					//发送起始信号

	IIC_SendData(RDA5820_ADDR);	    //发送写命令
	IIC_WaitAck();					//等待应答
	IIC_SendData(addr);				//发送读取地址
	IIC_WaitAck();					//等待应答

	IIC_Start();
	IIC_SendData(RDA5820_ADDR + 1);	//发送读命令
	IIC_WaitAck();					//等待应答
	dat = IIC_ReceiveData(1);        //读取高8位	，带应答
	dat <<= 8;
   	dat |= IIC_ReceiveData(0);		//读取低8位

	IIC_Stop();						//发送结束信号，不带应答

	return dat;
}

/****************************************************************************
* Function Name  : RDA5820_WriteData
* Description    : RDA5820使用IIC写入16位数据
* Input          : addr：写入寄存器地址
*                * dat：写入数据
* Output         : None
* Return         : None
****************************************************************************/

static void RDA5820_WriteData(uint8_t addr, uint16_t dat)
{
	IIC_Start();
	IIC_SendData(RDA5820_ADDR);	    //发送写命令
	IIC_WaitAck();					//等待应答
	IIC_SendData(addr);				//发送读取地址
	IIC_WaitAck();					//等待应答
	
	IIC_SendData(dat >> 8);		    //发送高8位数据
	IIC_WaitAck();					//等待应答
	IIC_SendData(dat & 0xFF);		//发送低8位数据
	IIC_WaitAck();					//等待应答

	IIC_Stop();						//发送结束信号 
}

/****************************************************************************
* Function Name  : RDA5820_Init
* Description    : 初始化RDA5820
* Input          : None
* Output         : None
* Return         : 0xFF：初始化失败；0：初始化成功
****************************************************************************/

uint8_t RDA5820_Init(void)
{
	uint16_t id;

	IIC_Config();
	id = RDA5820_ReadData(RDA5820_R00);
	if(id == 0x5805)
	{
	 	RDA5820_WriteData(RDA5820_R02, 0x0002);	//软复位
		RDA5820_DelayMs(50);
	 	RDA5820_WriteData(RDA5820_R02, 0xC001);	//立体声,上电
		RDA5820_DelayMs(600);				    //等待时钟稳定 
	 	RDA5820_WriteData(RDA5820_R05, 0X884F);	//搜索强度8,LNAN,1.8mA,VOL最大
 	 	RDA5820_WriteData(0x07, 0X7800);		// 
	 	RDA5820_WriteData(0x13, 0X0008);		// 
	 	RDA5820_WriteData(0x15, 0x1420);		//VCO设置  0x17A0/0x1420 
	 	RDA5820_WriteData(0x16, 0XC000);		//  
	 	RDA5820_WriteData(0x1C, 0X3126);		// 
	 	RDA5820_WriteData(0x22, 0X9C24);		//fm_true 
	 	RDA5820_WriteData(0x47, 0XF660);		//tx rds
        
        RDA5820_SetBand(0);                 //设置发送或者接受频率范围
    	RDA5820_SetSpace(2);                //设置频率步进大小
    	RDA5820_SetTxPGA(3);                //设置增益大小
    	RDA5820_SetTxPAG(63);               //发射功率大小
        return 0;
	}
    return 0xFF;
}

/****************************************************************************
* Function Name  : RDA5820_SetMode
* Description    : 设置RDA5820发送或者接受模式
* Input          : mode：设置模式
* Output         : None
* Return         : None
****************************************************************************/

void RDA5820_SetMode(uint8_t mode)
{
	uint16_t tmpreg;

	tmpreg = RDA5820_ReadData(RDA5820_R40);	 //读取初始设置
	
	tmpreg &= 0xFFF0;  //SHIP_FUNC[3:0]
	tmpreg |= mode;	   //有四种模式，定义在头文件中
	
	RDA5820_WriteData(RDA5820_R40, tmpreg);	 //设置
}

/****************************************************************************
* Function Name  : RDA5820_SetVol
* Description    : 设置声音的大小
* Input          : vol：大小设置（0x00~0x0F）
* Output         : None
* Return         : None
****************************************************************************/

void RDA5820_SetVol(uint8_t vol)
{
	uint16_t tmpreg;
	
	tmpreg = RDA5820_ReadData(RDA5820_R05);	 //读取初始设置
	
	tmpreg &= 0xFFF0;     //VOLUME[3:0]
	tmpreg |= vol & 0x0F; //从min:0000~max:1111	
	
	RDA5820_WriteData(RDA5820_R05, tmpreg);	 //设置	
}

/****************************************************************************
* Function Name  : RDA5820_GetVol
* Description    : 读取当前音量
* Input          : None
* Output         : None
* Return         : 读取到的音量值（最大16）
****************************************************************************/

uint8_t RDA5820_GetVol(void)
{
    uint16_t tmpreg;

    tmpreg = RDA5820_ReadData(RDA5820_R05);	 //读取初始设置

    tmpreg &= 0x000F;

    return (uint8_t)tmpreg;
}

/****************************************************************************
* Function Name  : RDA5820_SetBand
* Description    : 设置发送或接受频率范围
* Input          : band：设置频率范围band:0,87~108Mhz;1,76~91Mhz;2,76~108Mhz
* Output         : None
* Return         : None
****************************************************************************/

void RDA5820_SetBand(uint8_t band)
{
	uint16_t tmpreg;
	
	tmpreg = RDA5820_ReadData(RDA5820_R03);	 //读取初始设置

	tmpreg &= 0xFFF3;     // BAND是R03的2:3位 	
	tmpreg |= band << 2;  // band:0,87~108Mhz;1,76~91Mhz;2,76~108Mhz

	RDA5820_WriteData(RDA5820_R03, tmpreg);	 //设置
}

/****************************************************************************
* Function Name  : RDA5820_SetSpace
* Description    : 设置频率步进大小band:0,100Khz;1,200Khz;2,50Khz
* Input          : space：设置频率步进大小
* Output         : None
* Return         : None
****************************************************************************/

void RDA5820_SetSpace(uint8_t space)
{
	uint16_t tmpreg;

	tmpreg = RDA5820_ReadData(RDA5820_R03);	 //读取初始设置

	tmpreg &= 0XFFFC;	 //SPACE是BAND的[1:0]
	tmpreg |= space;	 //band:0,100Khz;1,200Khz;2,50Khz

	RDA5820_WriteData(RDA5820_R03, tmpreg);	 //设置
}

/****************************************************************************
* Function Name  : RDA5820_SetTxPGA
* Description    : 设置发送信号增益
* Input          : gain：设置增益大小，从min:000~max:111
* Output         : None
* Return         : None
****************************************************************************/

void RDA5820_SetTxPGA(uint8_t gain)
{
	uint16_t tmpreg;

	tmpreg = RDA5820_ReadData(RDA5820_R42);	 //读取初始设置
	
	tmpreg &= 0xF8FF;     //PGA[10:8]
	tmpreg |= gain << 8;  //从min:000~max:111	
	
	RDA5820_WriteData(RDA5820_R42, tmpreg);	 //设置		
}

/****************************************************************************
* Function Name  : RDA5820_SetTxPAG
* Description    : 设置发射功率
* Input          : gain：发射功率大小，从min:00000~max:11111
* Output         : None
* Return         : None
****************************************************************************/

void RDA5820_SetTxPAG(uint8_t gain)
{	
	uint16_t tmpreg;

	tmpreg = RDA5820_ReadData(RDA5820_R42);	 //读取初始设置
	
	tmpreg &= 0xFFC0;     //PA[5:0]
	tmpreg |= gain;       //从min:00000~max:11111	
	
	RDA5820_WriteData(RDA5820_R42, tmpreg);	 //设置	
}

/****************************************************************************
* Function Name  : RDA5820_SetSeekth
* Description    : 设置信号强度起始值
* Input          : seekth：设置值SEEKTH[14:8](设定信号强度)
* Output         : None
* Return         : None
****************************************************************************/

void RDA5820_SetSeekth(uint8_t seekth)
{
    uint16_t tmpreg;

    tmpreg = RDA5820_ReadData(RDA5820_R05);
    
    tmpreg &= 0x80FF;        // SEEKTH[14:8](设定信号强度)
    tmpreg |= ((uint16_t)seekth & 0x80) << 8;

    RDA5820_WriteData(RDA5820_R05, tmpreg);	 //设置
}

/****************************************************************************
* Function Name  : RDA5820_SetFrequency
* Description    : 设置RDA5820频率
* Input          : freq：频率大小，（频率为(freq / 100)MHZ）
* Output         : None
* Return         : None
****************************************************************************/

void RDA5820_SetFrequency(uint16_t freq)
{
	uint8_t space;
	uint16_t band, tmpreg, chan, i = 0;

	tmpreg = RDA5820_ReadData(RDA5820_R03);	 //读取初始设置
	tmpreg &= 0x001F;    //CHAN[15:8]
	band = (tmpreg >> 2) & 0x03;
	space = tmpreg & 0x03;

	/* 选择channel space */
	if(space == 0)	     //如果space=0,channel space is 100KHZ
	{
		space = 10;		
	}
	else if(space == 1)	 //如果space=1,channel space is 200KHZ
	{
		space = 20;	
	}
	else				 //如果space=2,channel space is 50KHZ
	{
		space = 5; 
	}

	/* 求当前设置的最低频率值 */
	if(band == 0)                    //band=0; 87~108MHZ
	{
		band = 8700;	
	}
	else if((band == 1)||(band == 2))//band=1; 76~91MHZ	band=2; 76~108MHZ
	{
		band = 7600;
	}
	else
	{
		band = RDA5820_ReadData(RDA5820_R53); //得到bottom频率
		band *= 10;
	}
	if(freq < band)
	{
		return;
	}
	
	/* chan = (frequence - chan_bottom(khz)) / channe spacing */
	chan = (freq - band) / space;
	chan &= 0x3FF;                 //取低10位
	tmpreg |= chan << 6;
	tmpreg |= 1 << 4;

	RDA5820_WriteData(RDA5820_R03, tmpreg);	 //设置频率	
	RDA5820_DelayMs(20);				 //延时20ms

	while((RDA5820_ReadData(RDA5820_R0B) & (1 << 7)) == 0) //等来FM ready
	{
		i++;
		if(i > 0xFFFE)
		{
			return;
		}	
	}
}

/****************************************************************************
* Function Name  : RDA5820_GetRssi
* Description    : 读取信号强度，RSSI[15:9](信号强度)
* Input          : None
* Output         : None
* Return         : 读取到的信号强度
****************************************************************************/

uint8_t RDA5820_GetRssi(void)
{
    uint16_t tmpreg;

    tmpreg = RDA5820_ReadData(RDA5820_R0B);	 //读取初始设置
    tmpreg >>= 8;

    return (uint8_t)tmpreg;
}

/****************************************************************************
* Function Name  : RDA5920_GetFrequency
* Description    : 读取接收频率
* Input          : None
* Output         : None
* Return         : 读取到的频率
****************************************************************************/

uint16_t  RDA5920_GetFrequency(void)
{
    uint8_t space;
	uint16_t band, tmpreg, freq;

	tmpreg = RDA5820_ReadData(RDA5820_R03);	 //读取初始设置
	
    freq = tmpreg >> 6;
	band = (tmpreg >> 2) & 0x03;
	space = tmpreg & 0x03;
    
    /* 选择channel space */
	if(space == 0)	     //如果space=0,channel space is 100KHZ
	{
		space = 10;		
	}
	else if(space == 1)	 //如果space=1,channel space is 200KHZ
	{
		space = 20;	
	}
	else				 //如果space=2,channel space is 50KHZ
	{
		space = 5; 
	}

    /* 求当前设置的最低频率值 */
	if(band == 0)                    //band=0; 87~108MHZ
	{
		band = 8700;	
	}
	else if((band == 1)||(band == 2))//band=1; 76~91MHZ	band=2; 76~108MHZ
	{
		band = 7600;
	}
	else
	{
		band = RDA5820_ReadData(RDA5820_R53); //得到bottom频率
		band *= 10;
	}
    freq = band + freq * space;
    
    return freq;
}



