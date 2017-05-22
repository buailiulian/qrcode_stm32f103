#include "dma.h"

/****************************************************************************
* Function Name  : DMA_Config
* Description    : 初始化DMA的设置
* Input          : DMA_CHx：设置的通道
*                * pAddr：外设基地址
*                * mAddr：发送数据内存基地址
*                * length：DMA缓存的大小
* Output         : None
* Return         : None
****************************************************************************/

void DMA_Config(DMA_Channel_TypeDef* DMA_CHx,uint32_t pAddr,uint32_t mAddr,uint16_t length)
{
    DMA_InitTypeDef DMA_InitStructure;

    /* 开启时钟 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
    
    DMA_DeInit(DMA_CHx);                    //将DMA的通道寄存器重设为缺省值

    /* 初始化设置 */
	DMA_InitStructure.DMA_PeripheralBaseAddr = pAddr;    //DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = mAddr;        //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //数据传输方向，从内存读取发送到外设
	DMA_InitStructure.DMA_BufferSize = length;          //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;   //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;            //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                    //工作在正常缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;            //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;           //DMA通道x没有设置为内存到内存传输
	
    DMA_Init(DMA_CHx, &DMA_InitStructure);                 //初始化
}

/****************************************************************************
* Function Name  : DMA_Enable
* Description    : 开启一次DMA传送
* Input          : DMA_CHx：使用的通道
*                * length：DMA缓存大小
* Output         : None
* Return         : None
****************************************************************************/

void DMA_Enable(DMA_Channel_TypeDef*DMA_CHx, uint32_t length)
{ 
	DMA_Cmd(DMA_CHx, DISABLE );                        //关闭USART1 TX DMA1 所指示的通道      
 	DMA_SetCurrDataCounter(DMA1_Channel4,length);      //DMA通道的DMA缓存的大小
 	DMA_Cmd(DMA_CHx, ENABLE);                          //使能USART1 TX DMA1 所指示的通道 
}
