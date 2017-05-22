#include "lcd_driver.h"
u16 tft_id;
/****************************************************************************
*函数名：TFT_GPIO_Config
*输  入：无
*输  出：无
*功  能：初始化TFT的IO口。
****************************************************************************/	  

void TFT_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* 打开时钟使能 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE  
	                      | RCC_APB2Periph_GPIOG, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	/* FSMC_A10(G12) 和RS（G0）*/
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 
	                              | GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_8 
								  | GPIO_Pin_9 | GPIO_Pin_10 |GPIO_Pin_11
								  | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14
								  | GPIO_Pin_15 );

	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9
	                               | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12
								   | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);

	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/****************************************************************************
* Function Name  : TFT_FSMC_Config
* Description    : 初始化FSMC
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void TFT_FSMC_Config(void)
{
	/* 初始化函数 */
	FSMC_NORSRAMInitTypeDef        FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  FSMC_NORSRAMTiming;

	/* 设置读写时序，给FSMC_NORSRAMInitStructure调用 */
	/* 地址建立时间，3个HCLK周期 */
	FSMC_NORSRAMTiming.FSMC_AddressSetupTime = 0x02;

	/* 地址保持时间，1个HCLK周期 */
	FSMC_NORSRAMTiming.FSMC_AddressHoldTime = 0x00;

	/* 数据建立时间，6个HCLK周期 */
	FSMC_NORSRAMTiming.FSMC_DataSetupTime = 0x05;

	/* 数据保持时间，1个HCLK周期 */
	FSMC_NORSRAMTiming.FSMC_DataLatency = 0x00;

	/* 总线恢复时间设置 */
	FSMC_NORSRAMTiming.FSMC_BusTurnAroundDuration = 0x00;
	
	/* 时钟分频设置 */
	FSMC_NORSRAMTiming.FSMC_CLKDivision = 0x01;

	/* 设置模式，如果在地址/数据不复用时，ABCD模式都区别不大 */
	FSMC_NORSRAMTiming.FSMC_AccessMode = FSMC_AccessMode_B;

	/*设置FSMC_NORSRAMInitStructure的数据*/
	/* FSMC有四个存储块（bank），我们使用第一个（bank1） */
	/* 同时我们使用的是bank里面的第 4个RAM区 */
	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;

	/* 这里我们使用SRAM模式 */
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;

	/* 使用的数据宽度为16位 */
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;

	/* 设置写使能打开 */
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;

	/* 选择拓展模式使能，即设置读和写用不同的时序 */
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable;
	
	/* 设置地址和数据复用使能不打开 */
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	
	/* 设置读写时序 */
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &FSMC_NORSRAMTiming;
	
	/* 设置写时序 */
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &FSMC_NORSRAMTiming;

	/* 打开FSMC的时钟 */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	 
	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

	/*!< Enable FSMC Bank1_SRAM Bank */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);	
}

/****************************************************************************
* Function Name  : TFT_WriteCmd
* Description    : LCD写入命令
* Input          : cmd：写入的16位命令
* Output         : None
* Return         : None
****************************************************************************/

void TFT_WriteCmd(uint16_t cmd)
{
	TFT->TFT_CMD = cmd >> 8;
    TFT->TFT_CMD = cmd & 0x00FF;
}

/****************************************************************************
* Function Name  : TFT_WriteData
* Description    : LCD写入数据
* Input          : dat：写入的16位数据
* Output         : None
* Return         : None
****************************************************************************/

void TFT_WriteData(u16 dat)
{
	TFT->TFT_DATA = dat >> 8;
    TFT->TFT_DATA = dat & 0x00FF;
}

/****************************************************************************
* Function Name  : TFT_Init
* Description    : 初始化LCD屏
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void TFT_Init(void)
{
	uint16_t i;

	TFT_GPIO_Config();
	TFT_FSMC_Config();

	for(i=500; i>0; i--);
	TFT_WriteCmd(0x0001);
	TFT_WriteData(0x0000);  //竖屏 0x0100	 横屏 0x0000
	TFT_WriteCmd(0x0003);
	TFT_WriteData(0x1038); //16bit 565 	  //竖屏显示0x1030    横屏显示0x1028
	TFT_WriteCmd(0x0008);
	TFT_WriteData(0x0808);
	TFT_WriteCmd(0x0090);
	TFT_WriteData(0x8000);
	TFT_WriteCmd(0x0400);
	TFT_WriteData(0x6200);
	TFT_WriteCmd(0x0401);
	TFT_WriteData(0x0001);

	TFT_WriteCmd(0x00ff);
	TFT_WriteData(0x0001);
	TFT_WriteCmd(0x0102);
	TFT_WriteData(0x01b0);
	TFT_WriteCmd(0x0710);
	TFT_WriteData(0x0016);
	TFT_WriteCmd(0x0712);
	TFT_WriteData(0x000f);
	TFT_WriteCmd(0x0752);
	TFT_WriteData(0x002f);
	TFT_WriteCmd(0x0724);
	TFT_WriteData(0x001a);
	TFT_WriteCmd(0x0754);	
	TFT_WriteData(0x0018);
	for(i=500; i>0; i--);
    
    /* Gamma */
	TFT_WriteCmd(0x0380);
	TFT_WriteData(0x0000);
	TFT_WriteCmd(0x0381);
	TFT_WriteData(0x5F10);//5f10
	TFT_WriteCmd(0x0382);
	TFT_WriteData(0x0B02);//0b02
	TFT_WriteCmd(0x0383);
	TFT_WriteData(0x0614);
	TFT_WriteCmd(0x0384);
	TFT_WriteData(0x0111);
	TFT_WriteCmd(0x0385);
	TFT_WriteData(0x0000);
	TFT_WriteCmd(0x0386);
	TFT_WriteData(0xA90B);//a90b
	TFT_WriteCmd(0x0387);
	TFT_WriteData(0x0606);//0606
	TFT_WriteCmd(0x0388);
	TFT_WriteData(0x0612);
	TFT_WriteCmd(0x0389);
	TFT_WriteData(0x0111);	  


    /* Gamma Setting */
    /* Setting */
	TFT_WriteCmd(0x0702);
	TFT_WriteData(0x003b);//003b
	TFT_WriteCmd(0x00ff);
	TFT_WriteData(0x0000);


    /* Vcom Setting */
	TFT_WriteCmd(0x0007);
	TFT_WriteData(0x0100);
	for(i=500; i>0; i--); 
	TFT_WriteCmd(0x0200);
	TFT_WriteData(0x0000);
	TFT_WriteCmd(0x0201);                  
	TFT_WriteData(0x0000);

}

/****************************************************************************
* Function Name  : TFT_SetWindow
* Description    : 设置要操作的窗口范围
* Input          : xStart：窗口起始X坐标
*                * yStart：窗口起始Y坐标
*                * xEnd：窗口结束X坐标
*                * yEnd：窗口结束Y坐标
* Output         : None
* Return         : None
****************************************************************************/

void TFT_SetWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd)
{
 	/*  					   //竖屏显示
	TFT_WriteCmd(0x0210);   
    TFT_WriteData(xStart);
	TFT_WriteCmd(0x0211);  
    TFT_WriteData(xEnd);
	TFT_WriteCmd(0x0212);   
    TFT_WriteData(yStart);
	TFT_WriteCmd(0x0213);   
    TFT_WriteData(yEnd);

	TFT_WriteCmd(0x0200);   
    TFT_WriteData(xStart);
	TFT_WriteCmd(0x0201);   
    TFT_WriteData(yStart);
	*/
	
	TFT_WriteCmd(0x0212);     //横屏显示
    TFT_WriteData(xStart);
	TFT_WriteCmd(0x0213);  
    TFT_WriteData(xEnd);
	TFT_WriteCmd(0x0210);   
    TFT_WriteData(yStart);
	TFT_WriteCmd(0x0211);   
    TFT_WriteData(yEnd);

	TFT_WriteCmd(0x0201);   
    TFT_WriteData(xStart);
	TFT_WriteCmd(0x0200);   
    TFT_WriteData(yStart);

	TFT_WriteCmd(0x0202);
}

/****************************************************************************
* Function Name  : TFT_ClearScreen
* Description    : 将LCD清屏成相应的颜色
* Input          : color：清屏颜色
* Output         : None
* Return         : None
****************************************************************************/
	  
void TFT_ClearScreen(uint16_t color)
{
 	uint16_t i, j ;

	TFT_SetWindow(0, 0, TFT_XMAX, TFT_YMAX);	 //作用区域
  	for(i=0; i<TFT_XMAX+1; i++)
	{
		for (j=0; j<TFT_YMAX+1; j++)
		{
			TFT_WriteData(color);
		}
	}
}

//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
	TFT_WriteCmd(0x0200);   
    TFT_WriteData(Xpos);
	TFT_WriteCmd(0x0201);   
    TFT_WriteData(Ypos);	
}

//读取个某点的颜色值	 
//x,y:坐标
//返回值:此点的颜色
u16 LCD_ReadPoint(u16 x,u16 y)
{
	u16 r=0,g=0,b=0;
	if(x>=240||y>=400)return 0;	//超过了范围,直接返回		   
	LCD_SetCursor(x,y);	    
	TFT_WriteCmd(0X0202);
	   
	if(TFT->TFT_DATA)r=0;							//dummy Read	   
	  
 	r=TFT->TFT_DATA;  		  						//实际坐标颜色
 		  
	b=TFT->TFT_DATA; 
	g=r&0XFF;		//对于9341/5310/5510,第一次读取的是RG的值,R在前,G在后,各占8位
	g<<=8;
		
	return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));//ILI9341/NT35310/NT35510需要公式转换一下
						
}
